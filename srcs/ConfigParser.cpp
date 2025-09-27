#include "ConfigParser.hpp"
#include "utils.hpp"
#include <cstdlib>

ConfigParser::ConfigParser(const std::string &file) : _file(file) {}

LocationConf	parse_log(size_t *i, std::vector<std::string> tokens, std::string root)
{
	LocationConf loc;

	loc.path = tokens[*i + 1];
	(*i) += 2;
	while ((*i) < tokens.size() && tokens[(*i)] != "}")
	{
		if (tokens[(*i)] == "methods")
		{
			loc.methods.clear();
			while (++(*i) < tokens.size() && tokens[(*i)][tokens[(*i)].size() - 1] != ';')
				loc.methods.push_back(tokens[(*i)]);
			if (tokens[(*i)][tokens[(*i)].size() - 1] == ';')
				loc.methods.push_back(strip_semicolon(tokens[(*i)]));
		}
		else if (tokens[(*i)] == "index")
		{
			loc.index_files.clear();
			while (++(*i) < tokens.size() && tokens[(*i)][tokens[(*i)].size() - 1] != ';')
				loc.index_files.push_back(tokens[(*i)]);
			if (tokens[(*i)][tokens[(*i)].size() - 1] == ';')
				loc.index_files.push_back(strip_semicolon(tokens[(*i)]));
		}
		else if (tokens[(*i)] == "autoindex")
			loc.autoindex = (strip_semicolon(tokens[++(*i)]) == "on");
		else if (tokens[(*i)] == "upload_dir")
			loc.upload_dir = strip_semicolon(tokens[++(*i)]);
		else if (tokens[(*i)] == "cgi")
			loc.cgi = (strip_semicolon(tokens[++(*i)]) == "on");
		else if (tokens[(*i)] == "cgi_extension")
			loc.cgi_extension = strip_semicolon(tokens[++(*i)]);
		else if (tokens[(*i)] == "root")
			loc.root = strip_semicolon(tokens[++(*i)]);
		++(*i);
	}
	if (loc.root.empty())
		loc.root = root;
	return (loc);
}

ServerConf ConfigParser::parse(void)
{
	std::ifstream ifs(_file.c_str());
	if (!ifs.is_open())
		throw std::runtime_error("Cannot open config file: " + _file);

	std::vector<std::string> tokens = tokenize(ifs);
	ServerConf conf;
	LocationConf loc;

	for (size_t i = 0; i < tokens.size() && i + 1 < tokens.size(); ++i)
	{
		if (tokens[i] == "listen")
		{
			std::string ip_port = strip_semicolon(tokens[i + 1]);
			size_t colon = ip_port.find(':');
			if (colon == std::string::npos)
				throw std::runtime_error("Invalid listen format : " + ip_port);

			std::string ip = ip_port.substr(0, colon);
			int port = atoi(ip_port.substr(colon + 1).c_str());
			conf.listen.push_back(std::make_pair(ip, port));
		}
		else if (tokens[i] == "root")
			conf.root = strip_semicolon(tokens[i + 1]);
		else if (tokens[i] == "error_page")
    		conf.error_pages[atoi(tokens[i + 1].c_str())] = conf.root + strip_semicolon(tokens[i + 2]);
		else if (tokens[i] == "client_max_body_size")
			conf.client_max_body_size = atoi(tokens[i + 1].c_str());
		else if (tokens[i] == "location")
			conf.locations.push_back(parse_log(&i, tokens, conf.root));
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
