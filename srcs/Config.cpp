#include "Config.hpp"
#include "utils.hpp"

/* constructor */
Config::Config() {}

/* destructor */
Config::~Config() {}

Config::Config(const Config &copy) : Server(copy)
{
	if (this != &copy)
		*this = copy;
}

Config						&Config::operator=(const Config &assignement)
{
	if (this != &assignement)
	{
		Servers = assignement.Servers;
		Sockets = assignement.Sockets;
	}
	return (*this);
}

/**
 * @brief : La fonction creer tous les sockets mentionne dans le fichier de config
**/
void	Config::CreateSocket(void)
{
	for (std::vector<Server>::iterator it = Servers.begin(); it != Servers.end(); it++)
	{
		std::vector<std::pair< std::string, int> > CurrentListen = it->getListen();//on recup le vector de ip/port a bind pour le serverblock actuelle 
		for (std::vector<std::pair< std::string, int> >::iterator it_current = CurrentListen.begin(); it_current != CurrentListen.end(); it_current++)//parcour le vector listen
		{
			Socket	*new_socket = new Socket(it_current->first, it_current->second, &(*it));
			this->Sockets.push_back(new_socket);
		}
	}
}

/**
 * @brief
 * Get the servers.
 * 
 * @return The vector of servers.
 */
const std::vector<Server>&	Config::getServers() const
{
	return Servers;
}

const std::vector<Socket *>&	Config::getSocket() const
{
	return (Sockets);
}

/**
 * @brief
 * Parse the configuration file `Location` part.
 * 
 * @param i The current increment on the `tokens`.
 * @param tokens The configuration file changed in tokens.
 * @param root The `Location` root path.
 * 
 * @return A `Location` structure that will be autmatically added to the corresponding `Server` structure.
 */
Locations	parse_loc(size_t *i, std::vector<std::string> tokens, std::string root)
{
	Locations loc;

	loc.path = tokens[*i + 1];
	*i += 2;
	while (*i < tokens.size() && tokens[*i] != "}")
	{
		if (tokens[*i] == "methods")
			fill_tokens(loc.methods, tokens, i);
		else if (tokens[*i] == "index")
			fill_tokens(loc.index_files, tokens, i);
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

/**
 * @brief
 * Parse a configuration file.
 * 
 * @param configFile The path to the configuration file.
 */
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
	//@brief : une fois la boucle finie avec tout les blocks servers: creer les socket avec un pointeur vers leurs servers blocks
	CreateSocket();
}

/**
 * @brief
 * Parse a server.
 * 
 * @param tokens A vector of tokens containing the configuration file informations.
 * @param i The `tokens` current increment.
 * 
 * @return A `Server` struct that will be automatically added in the vector of `Structs` of `Config` class.
 */
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
		else if (i + 1 < tokens.size())
		{
			if (tokens[i] == "listen")
			{
				std::string ip_port = strip_semicolon(tokens[i + 1]);
				size_t colon = ip_port.find(':');
				if (colon == std::string::npos)
					throw std::runtime_error("Invalid listen format : " + ip_port);
	
				std::string ip = ip_port.substr(0, colon);
				int port = atoi(ip_port.substr(colon + 1).c_str());
				conf.addListen(ip, port);
			}
			else if (tokens[i] == "root")
				conf.setRoot(strip_semicolon(tokens[i + 1]));
			else if (tokens[i] == "error_page" && i + 2 < tokens.size())
				conf.addErrorPage(atoi(tokens[i + 1].c_str()), conf.getRoot() + strip_semicolon(tokens[i + 2]));
			else if (tokens[i] == "client_max_body_size")
				conf.setClientMaxBodySize(atoi(tokens[i + 1].c_str()));
			else if (tokens[i] == "location")
				conf.addLocation(parse_loc(&i, tokens, conf.getRoot()));
		}
	}
	init_default_errors(conf);
	return conf;
}

/**
 * @brief
 * Convert a file stream into tokens.
 * 
 * @param ifs The ifstream type of value that will be tokenized.
 * 
 * @return A vector of string tokens.
 */
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
