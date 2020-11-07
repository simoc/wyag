#ifndef GIT_TAG_H
#define GIT_TAG_H

#include "GitCommit.h"

/**
 * \brief implements Tag objects in git Repository
 */
class GitTag : public GitCommit
{
public:
	GitTag(GitRepository *repo);
};

#endif
