#include <fstream>
#include <regex>

#include "ConfigParser.h"

std::string
ConfigParser::trim(const std::string &s)
{
	std::regex e("^\\s+|\\s+$");   // remove leading and trailing spaces
	return regex_replace(s, e, "");
}

void
ConfigParser::read(const std::string &filename)
{
	std::ifstream f(filename);
	if (f.is_open())
	{
		std::string line;
		std::string section;
		while (std::getline(f, line))
		{
			line = trim(line);
			if (line.size() > 0 && line.at(0) == '[')
			{
				auto s = line.substr(1);
				auto index = s.find(']');
				if (index != std::string::npos)
				{
					section = s.substr(0, index);
				}
			}
			else
			{
				auto index = line.find('=');
				if (index != std::string::npos)
				{
					auto key = trim(line.substr(0, index));
					auto value = trim(line.substr(index + 1));
					set(section, key, value);
				}
			}
		}
	}
}

std::string
ConfigParser::get(const std::string &section, const std::string &key)
{
	auto it = m_values.find(std::make_pair(section, key));
	if (it != m_values.end())
		return it->second;
	return std::string();
}

void
ConfigParser::set(const std::string &section, const std::string &key,
	const std::string &value)
{
	auto entry = std::make_pair(std::make_pair(section, key), value);
	m_values.insert(entry);
}

void
ConfigParser::write(const std::string &filename)
{
	std::ofstream f(filename);
	if (f.is_open())
	{
		std::vector<std::pair<std::string, std::string> > keys;
		for (auto const& entry : m_values)
	       	{
    			keys.push_back(entry.first);
		}
		std::sort(keys.begin(), keys.end());
		std::string section;
		for (auto const& key : keys)
		{
			if (key.first != section)
			{
				section = key.first;
				f << "[" << section << "]" << std::endl;
			}
			f << key.second << "=" << m_values.at(key) << std::endl;
		}
		f.close();
	}
}

