#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <vector>
#include <exception>
#include <memory>

#include "GitRepository.h"
#include "GitObject.h"
#include "GitCommit.h"
#include "GitTree.h"

int
cmd_init(const std::vector<std::string> &args)
{
	std::string path(".");
	if (args.size() > 2)
		path = args.at(2);

	GitRepository::repo_create(path);
	return(0);
}

int
cmd_cat_file(const std::vector<std::string> &args)
{
	int status = 0;
	std::string type;
	std::string sha;
	if (args.size() > 3)
	{
		type = args.at(2);
		sha = args.at(3);

		GitRepository repo = GitRepository::repo_find();
		auto obj = repo.object_read(sha);
		if (obj)
		{
			auto bytes = obj->serialize();
			for (const auto &ch : bytes)
			{
				std::cout << ch;
			}
		}
		else
		{
			std::cerr << "Not an object: " <<
				sha << std::endl;
			status = 1;
		}
	}
	else
	{
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" type object" << std::endl;
		status = 1;
	}
	return status;
}

int
cmd_hash_object(const std::vector<std::string> &args)
{
	int status = 0;
	std::string type("blob");
	bool write = false;
	size_t index = 2;
	while (index < args.size())
	{
		if (args.at(index) == "-w")
		{
			write = true;
			index++;
		}
		else if (args.at(index) == "-t" && index + 1 < args.size())
		{
			type = args.at(index + 1);
			index += 2;
		}
		else
		{
			break;
		}
	}
	if (index < args.size())
	{
		std::string filename = args.at(index);
		std::ifstream f(filename, std::ios::binary);
		if (f.is_open())
		{
			GitRepository repo = GitRepository::repo_find();
			std::string sha = repo.object_hash(f, type, write);
			std::cout << sha << std::endl;
		}
		else
		{
			std::cerr << "File not found: " << filename << std::endl;
			status = 1;
		}
	}
	else
	{
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" type object" << std::endl;
		status = 1;
	}
	return status;
}

int
log_graphviz(GitRepository &repo, const std::string &sha,
	std::set<std::string> &seen)
{
	if (seen.find(sha) != seen.end())
		return 0;

	seen.insert(sha);

	auto commit = repo.object_read(sha);
	if (commit == nullptr)
	{
		std::cerr << "Object not found: " << sha << std::endl;
		return 1;
	}
	if (commit->get_format() != "commit")
	{
		std::cerr << "Not a commit object: " << sha << std::endl;
		return 1;
	}
	auto parents = std::dynamic_pointer_cast<GitCommit>(commit)->get_value("parent");
	if (parents.empty())
	{
		// Base case: the initial commit.
		return 0;
	}

	for (const auto &p : parents)
	{
		std::cout << "c_" << sha << " -> c_" << p << ";" << std::endl;
		int status = log_graphviz(repo, p, seen);
		if (status != 0)
			return status;
	}

	return 0;
}

int
cmd_log(const std::vector<std::string> &args)
{
	int status = 0;
	if (args.size() > 2)
	{
		std::string commit = args.at(2);

		GitRepository repo = GitRepository::repo_find();

		std::cout << "digraph wyaglog{" << std::endl;
		std::set<std::string> seen;
		status = log_graphviz(repo,
			repo.object_find(commit), seen);
		if (status == 0)
			std::cout << "}" << std::endl;
	}
	else
	{
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" commit" << std::endl;
		status = 1;
	}
	return status;
}

int
cmd_ls_tree(const std::vector<std::string> &args)
{
	int status = 0;
	if (args.size() > 2)
	{
		std::string name = args.at(2);

		GitRepository repo = GitRepository::repo_find();
		auto obj = repo.object_read(repo.object_find(name, "tree"));
		if (obj == nullptr)
		{
			std::cerr << "Object not found: " << name << std::endl;
			return 1;
		}
		if (obj->get_format() != "tree")
		{
			std::cerr << "Not a tree object: " << name << std::endl;
			return 1;
		}
		auto items = std::dynamic_pointer_cast<GitTree>(obj)->get_items();
		for (const auto &item : items)
		{
			std::cout << std::setw(6) << std::setfill('0') <<
				item.mode;

			// Git's ls-tree displays the type
			// of the object pointed to.  We can do that too :)
			obj = repo.object_read(item.sha);
			if (obj == nullptr)
			{
				std::cerr << "Object not found: " << item.sha << std::endl;
				return 1;
			}
			std::cout << " " << obj->get_format() <<
				" " << item.sha << "\t" <<
				item.path << std::endl;
		}
	}
	else
	{
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" object" << std::endl;
		status = 1;
	}
	return status;
}

int
cmd_checkout(const std::vector<std::string> &args)
{
	int status = 0;
	if (args.size() > 3)
	{
		std::string commit = args.at(2);
		std::string path = args.at(3);

		GitRepository repo = GitRepository::repo_find();
		auto obj = repo.object_read(repo.object_find(commit, "tree"));
		if (obj == nullptr)
		{
			std::cerr << "Object not found: " << commit << std::endl;
			return 1;
		}
		// If the object is a commit, we grab its tree
		if (obj->get_format() == "commit")
		{
			auto tree = std::dynamic_pointer_cast<GitCommit>(obj)->get_value("tree");
			if (tree.empty())
			{
				std::cerr << "Tree object is empty: " << commit << std::endl;
				return 1;
			}
			std::string sha = tree.at(0);
			obj = repo.object_read(sha);
		}

		// Verify that path is an empty directory
		auto dir = fs::path(path);
		if (fs::exists(dir))
		{
			if (!fs::is_directory(dir))
			{
				std::cerr << "Not a directory: " << path << std::endl;
				return 1;
			}
			if (!fs::is_empty(dir))
			{
				std::cerr << "Directory not empty: " << path << std::endl;
				return 1;
			}
		}
		else
		{
			fs::create_directories(dir);
		}

		repo.tree_checkout(obj, path);
	}
	else
	{
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" commit path" << std::endl;
		status = 1;
	}
	return status;
}

int
show_ref(const std::map<std::string, GitRef> &refs,
	bool with_hash,
	const std::string &prefix)
{
	for (auto it = refs.begin(); it != refs.end(); ++it)
	{
		if (it->second.subref.empty() == false)
		{
			std::string prefix2 = prefix;
			if (prefix2.size() > 0)
				prefix2.append("/");
			prefix2.append(it->first);

			show_ref(it->second.subref, with_hash, prefix2);
		}
		else
		{
			if (with_hash)
			{
				std::cout << it->second.ref << " ";
			}
			if (prefix.size() > 0)
			{
				std::cout << prefix << "/";
			}
			std::cout << it->first << std::endl;
		}
	}
	return 0;
}

int
cmd_show_ref(const std::vector<std::string> &args)
{
	int status = 0;
	GitRepository repo = GitRepository::repo_find();
	auto refs = repo.ref_list();
	status = show_ref(refs, true, "refs");

	// More refs stored in packed-refs file
	auto packed_refs = repo.packed_ref_list();
	auto it = packed_refs.begin();
	while (it != packed_refs.end())
	{
		std::cout << it->second << " " << it->first << std::endl;
		++it;
	}
	return status;
}

int
cmd_tag(const std::vector<std::string> &args)
{
	int status = 0;
	GitRepository repo = GitRepository::repo_find();

	if (args.size() > 2)
	{
		// TODO implement tag_create() logic
	}
	else
	{
		auto refs = repo.ref_list();
		auto it = refs.find("tags");
		if (it != refs.end())
		{
			show_ref(it->second.subref, false, std::string());
		}

		// More refs stored in packed-refs file
		auto packed_refs = repo.packed_ref_list();
		auto it2 = packed_refs.begin();
		while (it2 != packed_refs.end())
		{
			if (it2->first.find("refs/tags/") == 0)
			{
				auto tag = it2->first.substr(10);
				std::cout << tag << std::endl;
			}
			++it2;
		}
	}
	return status;
}

int
process(const std::vector<std::string> &args)
{
	int status = 0;
	if (args.size() < 2)
	{
		std::cerr << "Usage: " << args.at(0) << " <command>" << std::endl;
		return(1);
	}

	std::string command = args.at(1);
	if (command == "init")
	{
		status = cmd_init(args);
	}
	else if (command == "cat-file")
	{
		status = cmd_cat_file(args);
	}
	else if (command == "hash-object")
	{
		status = cmd_hash_object(args);
	}
	else if (command == "log")
	{
		status = cmd_log(args);
	}
	else if (command == "ls-tree")
	{
		status = cmd_ls_tree(args);
	}
	else if (command == "checkout")
	{
		status = cmd_checkout(args);
	}
	else if (command == "show-ref")
	{
		status = cmd_show_ref(args);
	}
	else if (command == "tag")
	{
		status = cmd_tag(args);
	}
	else
	{
		std::cerr << "Unknown command: " << command << std::endl;
		status = 1;
	}
	return(status);
}

int
main(int argc, char *argv[])
{
	int status = 0;
	try
	{
		std::vector<std::string> args;
		for (int i = 0; i < argc; i++)
		{
			args.push_back(argv[i]);
		}
		status = process(args);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return status;
}
