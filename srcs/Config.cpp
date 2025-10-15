#include "Config.hpp"
#include "utils.hpp"
#include <cstdlib>

Config::Config() {}

Config::~Config() {}

Locations	parse_log(size_t *i, std::vector<std::string> tokens, std::string root)
{
	Locations loc;

	loc.path = tokens[*i + 1];
	*i += 2;
	while (*i < tokens.size() && tokens[*i] != "}")
	{
		if (tokens[*i] == "methods")
		{
			loc.methods.clear();
			while (++(*i) < tokens.size() && tokens[*i][tokens[*i].size() - 1] != ';')
				loc.methods.push_back(tokens[*i]);
			if (tokens[*i][tokens[*i].size() - 1] == ';')
				loc.methods.push_back(strip_semicolon(tokens[*i]));
		}
		else if (tokens[*i] == "index")
		{
			loc.index_files.clear();
			while (++(*i) < tokens.size() && tokens[*i][tokens[*i].size() - 1] != ';')
				loc.index_files.push_back(tokens[*i]);
			if (tokens[*i][tokens[*i].size() - 1] == ';')
				loc.index_files.push_back(strip_semicolon(tokens[*i]));
		}
		else if (tokens[*i] == "autoindex")
			loc.autoindex = (strip_semicolon(tokens[++(*i)]) == "on");
		else if (tokens[*i] == "upload_dir")
			loc.upload_dir = strip_semicolon(tokens[++(*i)]);
		else if (tokens[*i] == "cgi")
			loc.cgi = (strip_semicolon(tokens[++(*i)]) == "on");
		else if (tokens[*i] == "cgi_extension")
			loc.cgi_extension = strip_semicolon(tokens[++(*i)]);
		else if (tokens[*i] == "root")
			loc.root = strip_semicolon(tokens[++(*i)]);
		++(*i);
	}
	if (loc.root.empty())
		loc.root = root;
	return (loc);
}
const std::vector<Server>&	Config::getServers() const
{
	return Servers;
}

void Config::parseAllServerFiles(const std::string &configFile)
{
	std::ifstream ifs(configFile.c_str());
	if (!ifs.is_open())
		throw std::runtime_error("Cannot open config file : " + configFile);

	std::vector<std::string> tokens = tokenize(ifs);

	for (size_t i = 0; i < tokens.size(); ++i)
	{
		if (tokens[i] == "server")
		{
			try
			{
				if (i + 1 >= tokens.size() || tokens[i + 1] != "{")
					throw std::runtime_error("Expected '{' after 'server'");
				i += 2;
				if (i >= tokens.size())
					throw std::runtime_error("EOF after open brace");
				Server server = parse(tokens, i);
				Servers.push_back(server);
			}
			catch (const std::exception &e)
			{
				std::cerr << RED BOLD << "Error: " << e.what() << RESET << std::endl;
				return;
			}
		}
	}
}

Server Config::parse(const std::vector<std::string> &tokens, size_t &i)
{
	Server conf;
	int bracketCount = 1;

	for (; i < tokens.size() && bracketCount > 0; ++i)
	{
		if (tokens[i] == "{")
			++bracketCount;
		else if (tokens[i] == "}")
			--bracketCount;
		else if (tokens[i] == "listen" && i + 1 < tokens.size())
		{
			std::string ip_port = strip_semicolon(tokens[i + 1]);
			size_t colon = ip_port.find(':');
			if (colon == std::string::npos)
				throw std::runtime_error("Invalid listen format : " + ip_port);

			std::string ip = ip_port.substr(0, colon);
			int port = atoi(ip_port.substr(colon + 1).c_str());
			conf.addListen(ip, port);
		}
		else if (tokens[i] == "root" && i + 1 < tokens.size())
			conf.setRoot(strip_semicolon(tokens[i + 1]));
		else if (tokens[i] == "error_page" && i + 2 < tokens.size())
			conf.addErrorPage(atoi(tokens[i + 1].c_str()), conf.getRoot() + strip_semicolon(tokens[i + 2]));
		else if (tokens[i] == "client_max_body_size" && i + 1 < tokens.size())
			conf.setClientMaxBodySize(atoi(tokens[i + 1].c_str()));
		else if (tokens[i] == "location")
			conf.addLocation(parse_log(&i, tokens, conf.getRoot()));
	}
	init_default_errors(conf);
	return conf;
}

std::vector<std::string> Config::tokenize(std::istream &ifs)
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
