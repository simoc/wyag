#ifndef GIT_COMMIT_H
#define GIT_COMMIT_H

#include <map>

#include "GitObject.h"

/**
 * \brief implements Commit objects in git Repository
 */
class GitCommit : public GitObject
{
public:
	GitCommit(GitRepository *repo);

	std::vector<unsigned char> serialize();

	void deserialize(const std::vector<unsigned char> &data);

	std::vector<std::string> get_value(const std::string &key);

protected:
	GitCommit(GitRepository *repo, const std::string &fmt);

private:
	std::vector<unsigned char> m_blobdata;
	std::map<std::string, std::vector<std::string> > m_dct;

	std::string replace_all(const std::string &s,
		const std::string &before, const std::string &after);

	void kvlm_parse(const std::vector<unsigned char> &raw,
		size_t start,
		std::map<std::string, std::vector<std::string> > &dct);

	std::vector<unsigned char> kvlm_serialize();
};

#endif
