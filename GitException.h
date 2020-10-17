#ifndef GIT_EXCEPTION_H
#define GIT_EXCEPTION_H

#include <string>
#include <stdexcept>

class GitException : public std::runtime_error
{
public:
	GitException(const std::string &msg) :
		std::runtime_error(msg)
	{};
};

#endif
