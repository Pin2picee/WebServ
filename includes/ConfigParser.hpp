#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "Webserv.hpp"
#include <sstream>

#define RED     "\033[31m"

template<typename T>
std::string ft_to_string(T value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

struct LocationConf
{
	bool						cgi;
	bool						autoindex;
	std::string					root;
	std::string					path;
	std::string					upload_dir;
	std::string					cgi_extension;
	std::vector<std::string>	methods;
	std::vector<std::string>	index_files;
	
	LocationConf() : cgi(false), autoindex(false),
					 root(RED "none"), path(RED "none"),
					 upload_dir(RED "none"), cgi_extension(RED "none") {};
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

struct Request
{
	std::string							method;
    std::string							uri;
	std::string							path;
    std::string							body;
    std::map<std::string, std::string>	headers;

	Request() {};
};

struct Response
{
	int			status_code;
	std::string	content_type;
	std::string	body;

	Response() {};
};

Response handlePost(const LocationConf &loc, const Request &req, const ServerConf &conf);
Response handleDelete(const LocationConf &loc, const Request &req);

#endif
