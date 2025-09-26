#include "ConfigParser.hpp"
#include "Webserv.hpp"
#include <cstdlib>

ConfigParser::ConfigParser(const std::string &file) : _file(file) {}

void init_default_errors(ServerConf &conf)
{
	conf.error_pages[400] = conf.root + "/errors/400.html";
	conf.error_pages[401] = conf.root + "/errors/401.html";
	conf.error_pages[403] = conf.root + "/errors/403.html";
	conf.error_pages[404] = conf.root + "/errors/404.html";
	conf.error_pages[405] = conf.root + "/errors/405.html";
	conf.error_pages[413] = conf.root + "/errors/413.html";
	conf.error_pages[500] = conf.root + "/errors/500.html";
	conf.error_pages[501] = conf.root + "/errors/501.html";
	conf.error_pages[502] = conf.root + "/errors/502.html";
	conf.error_pages[503] = conf.root + "/errors/503.html";
}

ServerConf ConfigParser::parse(void)
{
	std::ifstream ifs(_file.c_str());
	if (!ifs.is_open())
		throw std::runtime_error("Cannot open config file: " + _file);

	std::vector<std::string> tokens = tokenize(ifs);
	ServerConf conf;
	conf.client_max_body_size = 1000000;
	conf.root = "/config/www/html";

	for (size_t i = 0; i < tokens.size(); ++i)
	{
		if (tokens[i] == "listen" && i + 1 < tokens.size())
		{
			std::string ip_port = tokens[i + 1];
			if (!ip_port.empty() && ip_port[ip_port.size() - 1] == ';')
				ip_port.resize(ip_port.size() - 1);
			size_t colon = ip_port.find(':');
			if (colon == std::string::npos)
				throw std::runtime_error("Invalid listen format : " + ip_port);

			std::string ip = ip_port.substr(0, colon);
			int port = atoi(ip_port.substr(colon + 1).c_str());
			conf.listen.push_back(std::make_pair(ip, port));
			break ;
		}
	}
	init_default_errors(conf);
	return conf;
}

std::vector<std::string> ConfigParser::tokenize(std::istream &ifs)
{
	std::vector<std::string> tokens;
	std::string line;

	while (std::getline(ifs, line))
	{
		std::istringstream iss(line);
		std::string tok;

		while (iss >> tok)
			tokens.push_back(tok);
	}
	return tokens;
}
