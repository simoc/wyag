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
	GitObject(GitRepository *repo, const std::string &fmt);

	virtual std::vector<unsigned char> serialize() = 0;

	virtual void deserialize(const std::vector<unsigned char> &data) = 0;

	std::string get_format() const;

private:
	GitRepository *m_repo;
	std::string m_fmt;
};

#endif
