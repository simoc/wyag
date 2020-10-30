#ifndef GIT_REF_H
#define GIT_REF_H

#include <string>
#include <map>

struct GitRef
{
	std::string ref;
	std::map<std::string, GitRef> subref;
};

#endif
