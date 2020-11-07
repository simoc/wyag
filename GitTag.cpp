#include "GitTag.h"

GitTag::GitTag(GitRepository *repo) :
	GitCommit(repo, "tag")
{
}
