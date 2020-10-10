#ifndef GIT_REPOSITORY_H
#define GIT_REPOSITORY_H

#include <string>
#include <vector>
#include <map>
#if __GNUC__ >= 9
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "ConfigParser.h"

class GitObject;

/*
 * \brief A git repository
 */
class GitRepository
{
public:
	GitRepository(const std::string &path, bool force = false);

	//! Create a new repository at path.
	static GitRepository repo_create(const std::string path);

	//! Search up through directory tree for repo's gitdir
	static GitRepository repo_find(const std::string &path = ".",
		bool required = true);

	//! Read object object_id from Git repository repo.
	GitObject *object_read(const std::string &sha);

	//! Write object to Git repository repo.
	std::string object_write(GitObject *obj, bool actually_write = true);

private:
	std::string m_worktree;
	fs::path m_gitdir;

	//! Compute path under repo's gitdir.
	fs::path repo_path(const std::string &path);

	//! Same as repo_path, but create dirname(path) if absent.
	fs::path repo_file(const std::string &path,
		bool mkdir = false);

	//! Same as repo_path, but mkdir(path) if absent.
	fs::path repo_dir(const std::string &path,
		bool mkdir = false);

	//! Get default configuration for new repository.
	ConfigParser repo_default_config();

	//! Compress bytes using zlib
	std::vector<unsigned char> compress_bytes(const std::vector<unsigned char> &bytes);

	//! Decompress zlib compressed bytes
	std::vector<unsigned char> uncompress_bytes(const std::vector<unsigned char> &bytes);

	//! Lookup table for object types
	static const std::map<std::string, std::string> m_object_types;
};

#endif
