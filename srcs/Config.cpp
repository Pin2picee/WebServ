#include "Config.hpp"
#include "utils.hpp"

/**
 * @brief
 * Default constructor for `Config`.
 */
Config::Config() {}

/**
 * @brief
 * Destructor for `Config`.
 */
Config::~Config() {}

/**
 * @brief
 * Copy constructor for `Config`.
 *
 * @param copy The `Config` instance to copy.
 */
Config::Config(const Config &copy) : Server(copy)
{
	if (this != &copy)
		*this = copy;
}

/**
 * @brief
 * Assignment operator for `Config`.
 *
 * @param assignement The `Config` instance to assign.
 *
 * @return Reference to the assigned `Config`.
 */
Config	&Config::operator=(const Config &assignement)
{
	if (this != &assignement)
	{
		Servers = assignement.Servers;
		Sockets = assignement.Sockets;
	}
	return (*this);
}

/**
 * @brief
 * Creates all sockets declared in the configuration file.
 *
 * Iterates through each server and its `listen` directives
 * to initialize the corresponding sockets.
 */
void	Config::CreateSocket(void)
{
	for (std::vector<Server>::iterator it = Servers.begin(); it != Servers.end(); it++)
	{
		std::vector<std::pair< std::string, int> > CurrentListen = it->getListen();//on recup le vector de ip/port a bind pour le serverblock actuelle
		for (std::vector<std::pair< std::string, int> >::iterator it_current = CurrentListen.begin();
			it_current != CurrentListen.end(); it_current++)//parcour le vector listen
		{
			Socket	*new_socket = new Socket(it_current->first, it_current->second, &(*it));
			this->Sockets.push_back(new_socket);
		}
	}
}

/**
 * @brief
 * Returns the vector of created sockets.
 *
 * @return A constant reference to the vector of `Socket*`.
 */
const std::vector<Socket *>&	Config::getSocket() const
{
	return (Sockets);
}

/**
 * @brief
 * Parses a `location` block from the configuration file.
 *
 * @param i Current index in the token vector.
 * @param tokens Vector containing all configuration tokens.
 * @param t_size Total number of tokens.
 * 
 * @return A `Locations` structure corresponding to the parsed block.
 */
Locations	parse_loc(size_t &i, std::vector<std::string> tokens, size_t &t_size)
{
	Locations loc;

	loc.path = tokens[i + 1];
	if (loc.path.empty())
		throw std::runtime_error("empty location path");
	i += 2;
	while (i < t_size && tokens[i] != "}")
	{
		if (tokens[i - 1] == "#")
		{
			i++;
			continue;
		}
		if (tokens[i] == "methods")
			fillTokens(loc.methods, tokens, i);
		else if (tokens[i] == "index")
			fillTokens(loc.index_files, tokens, i);
		else if (tokens[i] == "autoindex")
			loc.autoindex = (stripSemicolon(tokens[++i]) == "on");
		else if (tokens[i] == "upload_dir")
			loc.upload_dir = stripSemicolon(tokens[++i]);
		else if (tokens[i] == "cgi")
			loc.cgi = (stripSemicolon(tokens[++i]) == "on");
		else if (tokens[i] == "cgi_extension")
			loc.cgi_extension = stripSemicolon(tokens[++i]);
		else if (tokens[i] == "cgi_path")
			loc.cgi_path = stripSemicolon(tokens[++i]);
		else if (tokens[i] == "root")
			loc.root = stripSemicolon(tokens[++i]);
		else if (tokens[i] == "sensitive")
			loc.sensitive = (stripSemicolon(tokens[++i]) == "on");
		++i;
	}
	return (loc);
}

/**
 * @brief
 * Parses all server blocks from a configuration file.
 *
 * @param configFile Path to the configuration file.
 */
void Config::parseAllServerFiles(const std::string &configFile)
{
	std::ifstream ifs(configFile.c_str());
	if (!ifs.is_open())
		throw std::runtime_error("Cannot open config file : " + configFile);

	std::vector<std::string> tokens = tokenize(ifs);
	size_t size = tokens.size();

	for (size_t i = 0; i < size; ++i)
	{
		if ((i != 0 && tokens[i - 1][0] != '#' && tokens[i] == "server")
		|| (i == 0 && tokens[i] == "server"))
		{
			if (i + 1 >= size || tokens[i + 1] != "{")
				throw std::runtime_error("Expected '{' after 'server'");
			i += 2;
			if (i >= size)
				throw std::runtime_error("EOF after open brace");
			Server server = parse(tokens, size, i);
			Servers.push_back(server);
		}
	}
	CreateSocket();
}

/**
 * @brief
 * Parses a single server block.
 *
 * @param tokens Vector of configuration tokens.
 * @param t_size Total number of tokens.
 * @param i Current index in the token vector.
 * 
 * @return A fully initialized `Server` instance.
 */
Server Config::parse(const std::vector<std::string> &tokens, size_t &t_size, size_t &i)
{
	Server conf;
	int bracketCount = 1;

	for (; i < t_size && bracketCount > 0; ++i)
	{
		if (tokens[i] == "{")
			++bracketCount;
		else if (tokens[i] == "}")
			--bracketCount;
		else if (i + 1 < t_size)
		{
			if (i != 0 && tokens[i - 1] == "#")
				continue;
			if (tokens[i] == "listen")
			{
				std::string ip_port = stripSemicolon(tokens[i + 1]);
				size_t colon = ip_port.find(':');
				if (colon == std::string::npos)
					throw std::runtime_error("Invalid listen format : " + ip_port);
				std::string ip = ip_port.substr(0, colon);
				if (ip != "127.0.0.1" && ip != "localhost")
					throw std::runtime_error("IP adress should be \"127.0.0.1\" or \"localhost\", not " + ip);
				int port = atoi(ip_port.substr(colon + 1).c_str());
				conf.addListen(ip, port);
			}
			else if (tokens[i] == "root")
				conf.setRoot(stripSemicolon(tokens[i + 1]));
			else if (tokens[i] == "error_pages")
				conf.addErrorDir(stripSemicolon(tokens[i + 1]));
			else if (tokens[i] == "client_max_body_size")
				conf.setClientMaxBodySize(convertSize((tokens[i + 1])));
			else if (tokens[i] == "location")
				conf.addLocation(parse_loc(i, tokens, t_size));
		}
	}
	if (bracketCount)
		throw std::runtime_error("Server not closed properly");
	else if (conf.getErrorDir().empty())
		throw std::runtime_error("No error directory found");
	else if (conf.getRoot().empty())
		throw std::runtime_error("No root found");
	const std::vector<Locations> &locations = conf.getLocations();
	bool hasPost = false;
	for (size_t i = 0; i < locations.size(); ++i)
	{
		if (!locations[i].methods.size())
			throw std::runtime_error("No methods allowed on \"location " + locations[i].path + "\"");
		for (size_t j = 0; j < locations[i].methods.size(); ++j)
			if (locations[i].methods[j] == "POST")
				hasPost = true;
		if (hasPost)
			break;
	}
	if (!conf.getClientMaxBodySize() && hasPost)
		throw std::runtime_error("No client max body size found");
	init_default_errors(conf);
	return conf;
}

/**
 * @brief
 * Tokenizes a configuration file stream.
 *
 * Splits the input stream into whitespace-separated tokens.
 *
 * @param ifs Input stream of the configuration file.
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
