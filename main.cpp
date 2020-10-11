#include <iostream>
#include <string>
#include <exception>

#include "GitRepository.h"
#include "GitObject.h"

int
cmd_init(int argc, char *argv[])
{
	std::string path(".");
	if (argc > 2)
		path = argv[2];

	GitRepository::repo_create(path);
	return(0);
}

int
cmd_cat_file(int argc, char *argv[])
{
	int status = 0;
	std::string type;
	std::string sha;
	if (argc > 3)
	{
		type = argv[2];
		sha = argv[3];

		GitRepository repo = GitRepository::repo_find();
		auto obj = repo.object_read(sha);
		if (obj)
		{
			auto bytes = obj->serialize();
			for (const auto &ch : bytes)
			{
				std::cout << ch;
			}
		}
		else
		{
			std::cerr << "Not an object: " <<
				sha << std::endl;
			status = 1;
		}
	}
	else
	{
		std::cerr << "Usage: " << argv[0] << " " << argv[1] <<
			" type object" << std::endl;
		status = 1;
	}
	return status;
}

int
process(int argc, char *argv[])
{
	int status = 0;
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <command>" << std::endl;
		return(1);
	}

	std::string command = argv[1];
	if (command == "init")
	{
		status = cmd_init(argc, argv);
	}
	else if (command == "cat-file")
	{
		status = cmd_cat_file(argc, argv);
	}
	else
	{
		std::cerr << "Unknown command: " << command << std::endl;
		status = 1;
	}
	return(status);
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
