#include <exception>
#include <fstream>
#include <vector>

#include "GitRepository.h"
#include "ConfigParser.h"

GitRepository::GitRepository(const std::string &path, bool force)
{
	m_worktree = path;
	m_gitdir = fs::path(path) / ".git";

	if (!(force || fs::is_directory(m_gitdir)))
	{
		throw fs::filesystem_error("Not a git repository: " + m_gitdir.string(), std::error_code());
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
		throw fs::filesystem_error("Configuration file missing", std::error_code());
	}

	if (!force)
	{
		std::string vers = conf.get("core", "repositoryformatversion");
		if (vers != "0")
		{
			throw fs::filesystem_error("Unsupported repositoryformatversion: " + vers, std::error_code());
		}
	}
}

fs::path
GitRepository::repo_path(const std::string &path)
{
	return m_gitdir / path;
}

fs::path
GitRepository::repo_file(const std::string &path, bool mkdir)
{
	auto parentpath = fs::path(path).parent_path();
	auto fullpath = repo_dir(parentpath, mkdir);
	if (fullpath.empty())
		return fullpath;
	return repo_path(path);
}

fs::path
GitRepository::repo_dir(const std::string &path, bool mkdir)
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
			throw fs::filesystem_error("Not a directory: " + fullpath.string(), std::error_code());
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
			throw fs::filesystem_error(repo.m_worktree + " is not a directory", std::error_code());
		}
		if (!fs::is_empty(repo.m_worktree))
		{
			throw fs::filesystem_error(repo.m_worktree + " is not empty", std::error_code());
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
			throw fs::filesystem_error("Failed to create directory: " + dir, std::error_code());
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
		return GitRepository::repo_create(p.string());
	}

	// If we haven't returned, recurse in parent
	auto parentpath = p.parent_path();
	if (parentpath == p)
	{
		// Bottom case
		// parent of "/" is "/".
		// if parent == path, then path is root.
		if (required)
			throw fs::filesystem_error("Not a git directory ", std::error_code());
		else
			return GitRepository(std::string());
	}

	// Recursive case
	return repo_find(parentpath.string(), required);
}
