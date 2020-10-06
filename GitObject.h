#ifndef GIT_OBJECT_H
#define GIT_OBJECT_H

#include <vector>

#include "GitRepository.h"

/*
 * \brief Base class for an object in git Repository
 */
class GitObject
{
public:
	GitObject(GitRepository *repo);

	virtual std::vector<unsigned char> serialize() = 0;

	virtual void deserialize(const std::vector<unsigned char> &data) = 0;

private:
	GitRepository *m_repo;
};

#endif
