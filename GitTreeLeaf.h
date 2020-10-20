#ifndef GIT_TREE_LEAF_H
#define GIT_TREE_LEAF_H

#include <string>

/**
 * \brief A leaf node in a tree object.
 */
class GitTreeLeaf
{
public:
	GitTreeLeaf(const std::string &mode_,
		const std::string &path_,
		const std::string &sha_) :
		mode(mode_),
		path(path_),
		sha(sha_)
	{
	};

	std::string mode;
	std::string path;
	std::string sha;
};

#endif
