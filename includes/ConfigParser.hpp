#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>

struct LocationConf
{
	std::string					path;
	std::vector<std::string>	methods;
	std::string					root;
	bool						autoindex;
	std::string					upload_dir;
	bool						cgi_enabled;
};

struct ServerConf
{
	std::vector<std::pair<std::string, int> >	listen;
	std::string									root;
	std::map<int, std::string>					error_pages;
	size_t										client_max_body_size;
	std::vector<LocationConf>					locations;
};

class ConfigParser
{
private:
	std::string					_file;
	std::vector<std::string>	tokenize(std::istream &ifs);

public:
	ConfigParser(const std::string &file);
	ServerConf	 parse();
};

#endif
