#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <string>
#include <utility>
#include <map>

/*
 * \brief Reads and writes .ini configuration files
 */
class ConfigParser
{
public:
	void read(const std::string &filename);
	std::string get(const std::string &section, const std::string &key);

	void set(const std::string &section, const std::string &key,
		const std::string &value);
	void write(const std::string &filename);
private:
	std::map<std::pair<std::string, std::string>, std::string> m_values;

	std::string trim(const std::string &s);
};

#endif
