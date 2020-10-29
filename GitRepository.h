#ifndef GIT_REPOSITORY_H
#define GIT_REPOSITORY_H

#include <string>
#include <vector>
#include <memory>
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

/**
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
	std::shared_ptr<GitObject> object_read(const std::string &sha);

	//! Write object to Git repository repo.
	std::string object_write(std::shared_ptr<GitObject> obj, bool actually_write = true);

	//! Generate hash for file and optionally write file to repo.
	std::string object_hash(std::ifstream &f, const std::string &fmt, bool actually_write = false);

	std::string object_find(const std::string &name,
		const std::string &fmt = "",
		bool follow = true);

	//! Write tree object to empty directory.
	void tree_checkout(std::shared_ptr<GitObject> obj, const std::string &path);

private:
	std::string m_worktree;
	fs::path m_gitdir;
	//! ref to sha lookup table.
	std::map<std::string, std::string> m_packed_refs;

	//! Read all packed-refs into lookup table.
	void read_packed_refs(const std::string &path);

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
};

#endif
