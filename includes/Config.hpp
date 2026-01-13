#ifndef CONFIG_HPP
# define CONFIG_HPP

# include "Socket.hpp"
# include "Server.hpp"

/**
 * @brief
 * The class that Contains all the configuration file informations.
 * 
 * @param Servers				The list of servers in the configuration file.
 */

class Config : public Server
{
private:
	std::vector<Server>			Servers;
	std::vector<Socket * >		Sockets;

	std::vector<std::string>	tokenize(std::istream &ifs);
	Server						parse(const std::vector<std::string> &tokens, size_t &t_size, size_t &i);
	void						CreateSocket(void);

public:
	Config();
	~Config();
	Config(const Config& copy);
	Config						&operator=(const Config &assignement);
	
	//parsing
	void						parseAllServerFiles(const std::string &configFile);

	//getter
	const std::vector<Socket *>&		getSocket() const;
};

#endif
