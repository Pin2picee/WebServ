#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Socket.hpp"
# include "Server.hpp"

/**
 * @brief
 * Handles parsing and storage of the configuration file.
 * 
 * This class is responsible for:
 * - Parsing the configuration file
 * - Creating `Server` instances
 * - Creating and storing corresponding `Socket` instances
 * 
 * @param Servers	The list of servers defined in the configuration file.
 * @param Sockets	The list of sockets created from the configuration.
 */
class Config : public Server
{
private:
	std::vector<Server>			Servers;
	std::vector<Socket * >		Sockets;

	// Parsing utilities

	std::vector<std::string>	tokenize(std::istream &ifs);
	Server						parse(const std::vector<std::string> &tokens, size_t &t_size, size_t &i);
	void						CreateSocket(void);

public:
	Config();
	~Config();
	Config(const Config& copy);
	Config						&operator=(const Config &assignement);
	
	// Parsing

	void						parseAllServerFiles(const std::string &configFile);

	// Getter

	const std::vector<Socket *>	&getSocket() const;
};

#endif
