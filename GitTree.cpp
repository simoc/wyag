#include <algorithm>
#include <tuple>
#include <iomanip>
#include <sstream>

#include "GitTree.h"
#include "GitException.h"

GitTree::GitTree(GitRepository *repo) :
	GitObject(repo, "tree")
{
}

std::vector<unsigned char>
GitTree::serialize()
{
	std::vector<unsigned char> ret;
	for (const auto &i : m_items)
	{
		ret.insert(ret.end(), i.mode.begin(), i.mode.end());
		ret.push_back(' ');
		ret.insert(ret.end(), i.path.begin(), i.path.end());
		ret.push_back('\0');
		for (size_t j = 0; j < i.sha.size(); j += 2)
		{
			auto b = std::stoul(i.sha.substr(j, 2), 0, 16);
			ret.push_back(b & 0xff);
		}
	}
	return ret;
}

void
GitTree::deserialize(const std::vector<unsigned char> &data)
{
	m_items = tree_parse(data);
}

std::tuple<size_t, GitTreeLeaf>
GitTree::tree_parse_one(const std::vector<unsigned char> &raw, size_t start)
{
	// Find the space terminator of the mode
	unsigned char space = ' ';
	auto it1 = std::find(raw.begin() + start, raw.end(), space);
	if (it1 == raw.end())
		throw GitException("Not a tree object");
	size_t count = it1 - (raw.begin() + start);

	// Read the mode
	std::string mode;
	mode.append(raw.begin() + start, it1);

	// Find the NULL terminator of the path
	auto it2 = std::find(it1, raw.end(), '\0');
	if (it2 == raw.end())
		throw GitException("Not a tree object");
	// and read the path
	std::string path;
	path.append(it1 + 1, it2);

	// Skip zero byte
	it2++;

	// Read the SHA and convert to an hex string
	std::ostringstream hex_digits;
	int j = 0;
	while (j < 20 && it2 != raw.end())
	{
		hex_digits << std::hex << std::setw(2) <<
			std::setfill('0') <<
			static_cast<unsigned int>(*it2);
		it2++;
		j++;
	}
	count += (it2 - it1);
	return {start + count, GitTreeLeaf(mode, path, hex_digits.str())};
}

std::vector<GitTreeLeaf>
GitTree::tree_parse(const std::vector<unsigned char> &raw)
{
	size_t pos = 0;
	size_t len = raw.size();
	std::vector<GitTreeLeaf> ret;
	while (pos < len)
	{
		auto [newpos, data] = tree_parse_one(raw, pos);
		ret.push_back(data);
		pos = newpos;
	}
	return ret;
}

std::vector<GitTreeLeaf>
GitTree::get_items() const
{
	return m_items;
}
