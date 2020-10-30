#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "GitRepository.h"
#include "GitObject.h"
#include "GitBlob.h"
#include "GitCommit.h"
#include "GitTree.h"
#include "ConfigParser.h"
#include "GitException.h"

#include "zlib.h"
#include <openssl/sha.h>

GitRepository::GitRepository(const std::string &path, bool force)
{
	m_worktree = path;
	m_gitdir = fs::path(path) / ".git";

	if (!(force || fs::is_directory(m_gitdir)))
	{
		throw GitException("Not a git repository: " + m_gitdir.string());
	}

	// Read configuration file in .git/config
	ConfigParser conf;
	auto confpath = repo_file("config");
	if (fs::exists(confpath))
	{
		conf.read(confpath.string());
	}
	else if (!force)
	{
		throw GitException("Configuration file missing");
	}

	if (!force)
	{
		std::string vers = conf.get("core", "repositoryformatversion");
		if (vers != "0")
		{
			throw GitException("Unsupported repositoryformatversion: " + vers);
		}
	}

	auto packed_refs_path = repo_file("packed-refs");
	if (fs::exists(packed_refs_path))
	{
		read_packed_refs(packed_refs_path.string());
	}
}

void
GitRepository::read_packed_refs(const std::string &path)
{
	std::ifstream f(path);
	if (f.is_open())
	{
		std::string line;
		while (std::getline(f, line))
		{
			auto idx = line.find('#');
			if (idx == 0)
			{
				// skip comment lines
				continue;
			}

			idx = line.find(' ');
			if (idx != std::string::npos)
			{
				auto packed_sha = line.substr(0, idx);
				auto packed_ref = line.substr(idx + 1);
				m_packed_refs.insert({packed_ref, packed_sha});
			}
		}
	}
}

fs::path
GitRepository::repo_path(const std::string &path) const
{
	return m_gitdir / path;
}

fs::path
GitRepository::repo_file(const std::string &path, bool mkdir) const
{
	auto parentpath = fs::path(path).parent_path();
	auto fullpath = repo_dir(parentpath, mkdir);
	if (fullpath.empty())
		return fullpath;
	return repo_path(path);
}

fs::path
GitRepository::repo_dir(const std::string &path, bool mkdir) const
{
	auto fullpath = repo_path(path);
	if (fs::exists(fullpath))
	{
		if (fs::is_directory(fullpath))
		{
			return fullpath;
		}
		else
		{
			throw GitException("Not a directory: " + fullpath.string());
		}
	}

	if (mkdir)
	{
		fs::create_directories(fullpath);
		return fullpath;
	}
	return fs::path();
}

GitRepository
GitRepository::repo_create(const std::string path)
{
	auto repo = GitRepository(path, true);

	// First, we make sure the path either doesn't exist or is an
    	// empty dir.
	if (fs::exists(repo.m_worktree))
	{
		if (!fs::is_directory(repo.m_worktree))
		{
			throw GitException(repo.m_worktree + " is not a directory");
		}
		if (!fs::is_empty(repo.m_worktree))
		{
			throw GitException(repo.m_worktree + " is not empty");
		}
	}
	else
	{
		fs::create_directories(repo.m_worktree);
	}

	const std::vector<std::string> dirs = {"branches", "objects", "refs/tags", "refs/heads"};
	for (const auto &dir : dirs)
	{
		if (repo.repo_dir(dir, true).empty())
		{
			throw GitException("Failed to create directory: " + dir);
		}
	}

	std::ofstream fd(repo.repo_file("description").string());
	if (fd.is_open())
	{
		fd << "Unnamed repository; edit this file 'description' to name the repository." << std::endl;
		fd.close();
	}

	std::ofstream fh(repo.repo_file("HEAD").string());
	if (fh.is_open())
	{
		fh << "ref: refs/heads/master" << std::endl;
		fh.close();
	}

	ConfigParser config = repo.repo_default_config();
	config.write(repo.repo_file("config").string());

	return repo;
}

ConfigParser
GitRepository::repo_default_config()
{
	ConfigParser ret;
	ret.set("core", "repositoryformatversion", "0");
	ret.set("core", "filemode", "false");
	ret.set("core", "bare", "false");
	return ret;
}

GitRepository
GitRepository::repo_find(const std::string &path, bool required)
{
	auto p = fs::path(path);
	auto dir = p / ".git";
	if (fs::exists(dir))
	{
		return GitRepository(p.string());
	}

	// If we haven't returned, recurse in parent
	auto parentpath = p.parent_path();
	if (parentpath == p)
	{
		// Bottom case
		// parent of "/" is "/".
		// if parent == path, then path is root.
		if (required)
			throw GitException("Not a git directory ");
		else
			return GitRepository(std::string());
	}

	// Recursive case
	return repo_find(parentpath.string(), required);
}

std::vector<unsigned char>
GitRepository::compress_bytes(const std::vector<unsigned char> &bytes)
{
	std::vector<unsigned char> compressed;
	compressed.resize(bytes.size() * 2);

	uLong compressed_size = compressed.size();
	if (compress(compressed.data(), &compressed_size,
		bytes.data(), bytes.size()) == Z_OK)
	{
		compressed.resize(compressed_size);
	}
	else
	{
		compressed.clear();
	}

	return compressed;
}

std::vector<unsigned char>
GitRepository::uncompress_bytes(const std::vector<unsigned char> &bytes)
{
	std::vector<unsigned char> uncompressed;
	uncompressed.resize(bytes.size() * 10);

	uLong uncompressed_size = uncompressed.size();
	int status = uncompress(uncompressed.data(), &uncompressed_size,
		bytes.data(), bytes.size());
	if (status == Z_OK)
	{
		uncompressed.resize(uncompressed_size);
	}
	else if (status == Z_BUF_ERROR)
	{
		// Try uncompressing again with a bigger buffer
		uncompressed.resize(bytes.size() * 100);
		status = uncompress(uncompressed.data(), &uncompressed_size,
			bytes.data(), bytes.size());
		if (status == Z_OK)
		{
			uncompressed.resize(uncompressed_size);
		}
		else
		{
			uncompressed.clear();
		}
	}
	else
	{
		uncompressed.clear();
	}

	return uncompressed;
}

std::shared_ptr<GitObject>
GitRepository::object_read(const std::string &sha)
{
	std::vector<unsigned char> bytes;
	if (sha.size() >= 2)
	{
		std::string objpath = "objects/" +
			sha.substr(0, 2) + "/" + sha.substr(2);

		auto path = repo_file(objpath);
		std::ifstream f(path.string(), std::ios::binary);
		if (f.is_open())
		{
			unsigned char ch = f.get();
			while (f.good())
			{
				bytes.push_back(ch);
				ch = f.get();
			}
			f.close();

			bytes = uncompress_bytes(bytes);
			auto it1 = std::find(bytes.begin(), bytes.end(), ' ');
			if (it1 != bytes.end())
			{
				std::string fmt(bytes.begin(), it1);
				auto it2 = std::find(it1, bytes.end(), '\0');
				std::string size;
				std::vector<unsigned char> data;
				if (it2 != bytes.end())
				{
					size = std::string(it1 + 1, it2);
					data = std::vector<unsigned char>(it2 + 1, bytes.end());
				}

				if (fmt == "blob")
				{
					std::shared_ptr<GitObject> obj(new GitBlob(this));
					obj->deserialize(data);
					return obj;
				}
				else if (fmt == "commit")
				{
					std::shared_ptr<GitObject> obj(new GitCommit(this));
					obj->deserialize(data);
					return obj;
				}
				else if (fmt == "tree")
				{
					std::shared_ptr<GitObject> obj(new GitTree(this));
					obj->deserialize(data);
					return obj;
				}
				else
				{
					std::cerr << "fmt: " << fmt << std::endl;
				}
			}
		}
	}
	return nullptr;
}

std::string
GitRepository::object_write(std::shared_ptr<GitObject> obj, bool actually_write)
{
	// Serialize object data
	std::vector<unsigned char> data = obj->serialize();

	// Add header
	std::string header = obj->get_format() + " " + std::to_string(data.size());
	std::vector<unsigned char> result;
	for (const char &c : header)
	{
		result.push_back(c);
	}
	result.push_back('\0');
	for (const char &c : data)
	{
		result.push_back(c);
	}

	// Compute hash
	std::ostringstream hex_digits;
	unsigned char md[SHA_DIGEST_LENGTH] = {0};
	SHA1(result.data(), result.size(), md);
	for (size_t i = 0; i < sizeof(md); i++)
	{
		hex_digits << std::hex << std::setw(2) <<
			std::setfill('0') << static_cast<unsigned int>(md[i]);
	}
	std::string sha = hex_digits.str();

	if (actually_write)
	{
		// Compute path
		std::string objpath = "objects/" +
			sha.substr(0, 2) + "/" + sha.substr(2);
		auto path = repo_file(objpath, actually_write);

		std::ofstream f(path.string(), std::ios::binary);
		if (f.is_open())
		{
			auto bytes = compress_bytes(result);
			f.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
			f.close();
		}
	}

	return sha;
}

std::string
GitRepository::object_hash(std::ifstream &f, const std::string &fmt,
	bool actually_write)
{
	std::string sha;
	if (f.is_open())
	{
		std::vector<unsigned char> bytes;
		unsigned char ch = f.get();
		while (f.good())
		{
			bytes.push_back(ch);
			ch = f.get();
		}

		if (fmt == "blob")
		{
			std::shared_ptr<GitObject> obj(new GitBlob(this));
			obj->deserialize(bytes);
			sha = object_write(obj, actually_write);
		}
		else if (fmt == "commit")
		{
			std::shared_ptr<GitObject> obj(new GitCommit(this));
			obj->deserialize(bytes);
			sha = object_write(obj, actually_write);
		}
		else if (fmt == "tree")
		{
			std::shared_ptr<GitObject> obj(new GitTree(this));
			obj->deserialize(bytes);
			sha = object_write(obj, actually_write);
		}
		else
		{
			//TODO implement other object types when available
			std::cerr << "fmt: " << fmt << std::endl;
		}
	}
	return sha;
}

std::string
GitRepository::object_find(const std::string &name,
	const std::string &fmt,
	bool follow)
{
	return name;
}

void
GitRepository::tree_checkout(std::shared_ptr<GitObject> obj, const std::string &path)
{
	if (obj->get_format() != "tree")
		return;

	auto tree = std::dynamic_pointer_cast<GitTree>(obj);
	for (const auto &item : tree->get_items())
	{
		auto obj2 = object_read(item.sha);
		auto dest = fs::path(path) / item.path;

		if (obj2 == nullptr)
		{
			std::cerr << "Object not found: " << item.sha << std::endl;
		}
		else if (obj2->get_format() == "tree")
		{
			fs::create_directories(dest);
			tree_checkout(obj2, dest.string());
		}
		else if (obj2->get_format() == "blob")
		{
			std::ofstream f(dest.string(), std::ios::binary);
			if (f.is_open())
			{
				auto blobdata = obj2->serialize();
				f.write(reinterpret_cast<char *>(blobdata.data()), blobdata.size());
			}
		}
	}
}

std::string
GitRepository::ref_resolve(const std::string &ref) const
{
	std::string line;

	// Is it a packed ref?
	auto it = m_packed_refs.find(ref);
	if (it != m_packed_refs.end())
	{
		return it->second;
	}

	// Accept ref with or without directory path
	std::ifstream f(ref);
	if (!f.is_open())
	{
		auto refpath = repo_file(ref);
		f.open(refpath.string());
	}
	if (f.is_open())
	{
		std::getline(f, line);
		auto idx = line.find_last_of('\n');
		if (idx != std::string::npos && idx + 1 == line.size())
		{
			// Drop final \n ^^^^^
			line = line.substr(0, idx);
		}

		if (line.find("ref: ") == 0)
		{
			line = ref_resolve(line.substr(5));
		}
	}
	return line;
}

std::map<std::string, std::string>
GitRepository::packed_ref_list() const
{
	return m_packed_refs;
}

std::map<std::string, GitRef>
GitRepository::ref_list(const std::string &path) const
{
	std::map<std::string, GitRef> ret;

	std::string path2;
	if (path.empty())
	{
		path2 = repo_dir("refs");
	}
	else
	{
		path2 = path;
	}

	for(auto &f : fs::directory_iterator(path2))
	{
		GitRef entry;
		auto can = f.path();
		if (fs::is_directory(can))
		{
			if (!fs::is_empty(can))
			{
				entry.subref = ref_list(can.string());
				ret[f.path().filename().string()] = entry;
			}
		}
		else
		{
			entry.ref = ref_resolve(can.string());
			ret[f.path().filename().string()] = entry;
		}
	}
	return ret;
}
