#include <iostream>

#include "GitRepository.h"

int
main(int argc, char *argv[])
{
	GitRepository repo = GitRepository::repo_create(argv[1]);

	std::cout << "hello world!" << std::endl;
}
