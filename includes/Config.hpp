#ifndef CONFIG_HPP
# define CONFIG_HPP

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

	std::vector<std::string>	tokenize(std::istream &ifs);
	Server						parse(const std::vector<std::string> &tokens, size_t &i);
public:
	Config();
	~Config();

	//parsing

	void						parseAllServerFiles(const std::string &configFile);

	//getter

	const std::vector<Server>&	getServers() const;
};

#endif
