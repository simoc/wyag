#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <exception>
#include <memory>

#include "GitRepository.h"
#include "GitObject.h"
#include "GitCommit.h"

int
cmd_init(const std::vector<std::string> &args)
{
	std::string path(".");
	if (args.size() > 2)
		path = args.at(2);

	GitRepository::repo_create(path);
	return(0);
}

int
cmd_cat_file(const std::vector<std::string> &args)
{
	int status = 0;
	std::string type;
	std::string sha;
	if (args.size() > 3)
	{
		type = args.at(2);
		sha = args.at(3);

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
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" type object" << std::endl;
		status = 1;
	}
	return status;
}

int
cmd_hash_object(const std::vector<std::string> &args)
{
	int status = 0;
	std::string type("blob");
	bool write = false;
	size_t index = 2;
	while (index < args.size())
	{
		if (args.at(index) == "-w")
		{
			write = true;
			index++;
		}
		else if (args.at(index) == "-t" && index + 1 < args.size())
		{
			type = args.at(index + 1);
			index += 2;
		}
		else
		{
			break;
		}
	}
	if (index < args.size())
	{
		std::string filename = args.at(index);
		std::ifstream f(filename, std::ios::binary);
		if (f.is_open())
		{
			GitRepository repo = GitRepository::repo_find();
			std::string sha = repo.object_hash(f, type, write);
			std::cout << sha << std::endl;
		}
		else
		{
			std::cerr << "File not found: " << filename << std::endl;
			status = 1;
		}
	}
	else
	{
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" type object" << std::endl;
		status = 1;
	}
	return status;
}

int
log_graphviz(GitRepository &repo, const std::string &sha,
	std::set<std::string> &seen)
{
	if (seen.find(sha) != seen.end())
		return 0;

	seen.insert(sha);

	auto commit = repo.object_read(sha);
	if (commit == nullptr)
	{
		std::cerr << "Object not found: " << sha << std::endl;
		return 1;
	}
	if (commit->get_format() != "commit")
	{
		std::cerr << "Not a commit object: " << sha << std::endl;
		return 1;
	}
	auto parents = std::dynamic_pointer_cast<GitCommit>(commit)->get_value("parent");
	if (parents.empty())
	{
		// Base case: the initial commit.
		return 0;
	}

	for (const auto &p : parents)
	{
		std::cout << "c_" << sha << " -> c_" << p << ";" << std::endl;
		int status = log_graphviz(repo, p, seen);
		if (status != 0)
			return status;
	}

	return 0;
}

int
cmd_log(const std::vector<std::string> &args)
{
	int status = 0;
	if (args.size() > 2)
	{
		std::string commit = args.at(2);

		GitRepository repo = GitRepository::repo_find();

		std::cout << "digraph wyaglog{" << std::endl;
		std::set<std::string> seen;
		status = log_graphviz(repo,
			repo.object_find(commit), seen);
		if (status == 0)
			std::cout << "}" << std::endl;
	}
	else
	{
		std::cerr << "Usage: " << args.at(0) << " " << args.at(1) <<
			" commit" << std::endl;
		status = 1;
	}
	return status;
}

int
process(const std::vector<std::string> &args)
{
	int status = 0;
	if (args.size() < 2)
	{
		std::cerr << "Usage: " << args.at(0) << " <command>" << std::endl;
		return(1);
	}

	std::string command = args.at(1);
	if (command == "init")
	{
		status = cmd_init(args);
	}
	else if (command == "cat-file")
	{
		status = cmd_cat_file(args);
	}
	else if (command == "hash-object")
	{
		status = cmd_hash_object(args);
	}
	else if (command == "log")
	{
		status = cmd_log(args);
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
		std::vector<std::string> args;
		for (int i = 0; i < argc; i++)
		{
			args.push_back(argv[i]);
		}
		status = process(args);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return status;
}
