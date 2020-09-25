#include <iostream>
#include <string>
#include <exception>

#include "GitRepository.h"

void
cmd_init(int argc, char *argv[])
{
	std::string path(".");
	if (argc > 2)
		path = argv[2];

	GitRepository::repo_create(path);
}

int
process(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <command>" << std::endl;
		return(1);
	}

	std::string command = argv[1];
	if (command == "init")
	{
		cmd_init(argc, argv);
	}
	else
	{
		std::cerr << "Unknown command: " << command << std::endl;
		return(1);
	}
	return(0);
}

int
main(int argc, char *argv[])
{
	int status = 0;
	try
	{
		status = process(argc, argv);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return status;
}
