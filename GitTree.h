#ifndef GIT_TREE_H
#define GIT_TREE_H

#include "GitObject.h"
#include "GitTreeLeaf.h"

/**
 * \brief implements Tree objects in git Repository
 */
class GitTree : public GitObject
{
public:
	GitTree(GitRepository *repo);

	std::vector<unsigned char> serialize();

	void deserialize(const std::vector<unsigned char> &data);

	std::vector<GitTreeLeaf> get_items() const;
private:

	std::tuple<size_t, GitTreeLeaf>
		tree_parse_one(const std::vector<unsigned char> &raw, size_t start = 0);

	std::vector<GitTreeLeaf>
		tree_parse(const std::vector<unsigned char> &raw);

	std::vector<GitTreeLeaf> m_items;
};

#endif
