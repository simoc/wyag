#include "GitObject.h"

GitObject::GitObject(GitRepository *repo, const std::string &fmt)
{
	m_repo = repo;
	m_fmt = fmt;
}

std::string
GitObject::get_format() const
{
	return m_fmt;
}
