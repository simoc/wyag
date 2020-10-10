#ifndef GIT_BLOB_H
#define GIT_BLOB_H

#include "GitObject.h"

/*
 * \brief implements Blob objects in git Repository
 */
class GitBlob : public GitObject
{
public:
	GitBlob(GitRepository *repo);

	std::vector<unsigned char> serialize();

	void deserialize(const std::vector<unsigned char> &data);

private:
	std::vector<unsigned char> m_blobdata;
};

#endif
