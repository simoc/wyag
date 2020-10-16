#include <algorithm>

#include "GitCommit.h"

GitCommit::GitCommit(GitRepository *repo) :
	GitObject(repo, "commit")
{
}

std::vector<unsigned char>
GitCommit::serialize()
{
	return kvlm_serialize();
}

void
GitCommit::deserialize(const std::vector<unsigned char> &data)
{
	m_dct.clear();
	kvlm_parse(data, 0, m_dct);
}

std::vector<std::string>
GitCommit::get_value(const std::string &key)
{
	auto it = m_dct.find(key);
	if (it == m_dct.end())
		return std::vector<std::string>();
	return it->second;
}

std::string
GitCommit::replace_all(const std::string &s,
	const std::string &before, const std::string &after)
{
	std::string s2 = s;
	size_t idx = s2.find(before);
	while (idx != std::string::npos)
	{
		s2.replace(idx, before.size(), after);
		idx = s2.find(before, idx + after.size());
	}
	return s2;
}

void
GitCommit::kvlm_parse(const std::vector<unsigned char> &raw, size_t start,
	std::map<std::string, std::vector<std::string> > &dct)
{
	// We search for the next space and the next newline.
	int spc = -1;
	int nl = -1;

	const char space = ' ';
	const char newline = '\n';
	auto it1 = std::find(raw.begin() + start, raw.end(), space);
	if (it1 != raw.end())
	{
		spc = it1 - raw.begin();
	}
	auto it2 = std::find(raw.begin() + start, raw.end(), newline);
	if (it2 != raw.end())
	{
		nl = it2 - raw.begin();
	}

	// If space appears before newline, we have a keyword.

	// Base case
	// =========
	// If newline appears first (or there's no space at all, in which
	// case find returns -1), we assume a blank line.  A blank line
	// means the remainder of the data is the message.
	if ((spc < 0) || (nl < spc))
	{
		std::string value;
		value.append(raw.begin() + start + 1, raw.end());
		std::vector values{value};
		dct.insert({std::string(), values});
		return;
	}

	//  Recursive case
	// ==============
	// we read a key-value pair and recurse for the next.
	std::string key;
	key.append(raw.begin() + start, raw.begin() + spc);

	// Find the end of the value.  Continuation lines begin with a
	// space, so we loop until we find a '\n' not followed by a space.
	size_t ending = start;
	while (true)
	{
		auto it3 = std::find(raw.begin() + ending + 1,
			raw.end(), newline);
		if (it3 == raw.end())
			break;
		ending = it3 - raw.begin();
		if (*(it3 + 1) != space)
		{
			break;
		}
	}

	// Grab the value
	// Also, drop the leading space on continuation lines
	std::string value;
	value.append(raw.begin() + spc + 1, raw.begin() + ending);
	value = replace_all(value, "\n ", "\n");

	// Don't overwrite existing data contents
	auto it4 = dct.find(key);
	if (it4 != dct.end())
	{
		it4->second.push_back(value);
	}
	else
	{
		std::vector values{value};
		dct.insert({key, values});
	}

	kvlm_parse(raw, ending + 1, dct);
}

std::vector<unsigned char>
GitCommit::kvlm_serialize()
{
	std::vector<unsigned char> ret;

	std::vector<std::string> message;
	for (auto it = m_dct.begin(); it != m_dct.end(); it++)
	{
		std::string key = it->first;
		if (key.empty())
		{
			// Skip the message itself
			message = it->second;
			continue;
		}
		std::vector<std::string> val = it->second;
		for (const std::string &v : val)
		{
			for (const char &c : key)
			{
				ret.push_back(c);
			}
			ret.push_back(' ');

			std::string v2 = replace_all(v, "\n", "\n ");

			for (const char &c : v2)
			{
				ret.push_back(c);
			}
		}
	}

	// Append message
	ret.push_back('\n');
	for (const std::string &v : message)
	{
		for (const char &c : v)
		{
			ret.push_back(c);
		}
	}
	return ret;
}
