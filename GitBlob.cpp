#include "GitBlob.h"

GitBlob::GitBlob(GitRepository *repo) :
	GitObject(repo, "blob")
{
}

std::vector<unsigned char>
GitBlob::serialize()
{
	return m_blobdata;
}

void
GitBlob::deserialize(const std::vector<unsigned char> &data)
{
	m_blobdata = data;
}
