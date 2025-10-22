/* int main(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << RED BOLD << "Usage : ./webserv [configuration file]" RESET << std::endl;
		return 1;
	}

	std::string configFile = (ac == 2) ? av[1] : "config/default.conf";

	try
	{
		std::cout << CYAN BOLD << "📄 Configuration file loaded : " 
				  << configFile << RESET << std::endl;

		Config parser;
		parser.parseAllServerFiles(configFile);

		const std::vector<Server> &servers = parser.getServers();

		std::cout << GREEN BOLD << "\n🌐 Configured servers total : " 
				  << servers.size() << RESET << std::endl;

		// --- Each server content display ---
		for (size_t s = 0; s < servers.size(); ++s)
		{
			const Server &serv = servers[s];
			std::cout << MAGENTA BOLD << "\n==============================" << RESET << std::endl;
			std::cout << YELLOW BOLD << "🖥️  Server #" << s + 1 << RESET << std::endl;

			// ROOT
			std::cout << BOLD BLUE "Root : " RESET << serv.getRoot() << std::endl;

			// LISTEN
			std::cout << BOLD GREEN "Listen : " RESET;
			for (size_t j = 0; j < serv.getListen().size(); ++j)
			{
				std::cout << serv.getListen()[j].first << ":" 
						  << serv.getListen()[j].second << RESET;
				if (j + 1 < serv.getListen().size())
					std::cout << ", ";
			}
			std::cout << std::endl;

			// MAX BODY SIZE
			std::cout << BOLD YELLOW "Max body size : " RESET 
					  << serv.getClientMaxBodySize() << std::endl;

			// ERROR PAGES
			std::cout << BOLD RED "Error pages :" RESET << std::endl;
			for (std::map<int, std::string>::const_iterator it = serv.getErrorPages().begin();
				 it != serv.getErrorPages().end(); ++it)
			{
				std::cout << "  " << it->first << " => " << it->second << std::endl;
			}
			std::cout << std::endl;

			// LOCATIONS
			std::cout << BOLD MAGENTA "Locations : " RESET << serv.getLocations().size() << std::endl;
			for (size_t l = 0; l < serv.getLocations().size(); ++l)
			{
				const Locations &loc = serv.getLocations()[l];
				std::cout << "  " CYAN "- Location #" << l + 1 << RESET << std::endl;
				std::cout << "	" BOLD "Path : " RESET << BLUE << loc.path << RESET << std::endl;
				std::cout << "	" BOLD "Root : " RESET << BLUE << loc.root << RESET << std::endl;
				std::cout << "	" BOLD "Autoindex : " RESET 
						  << (loc.autoindex ? GREEN "on" RESET : RED "off" RESET) 
						  << std::endl;
			}
		}

		std::cout << GREEN BOLD 
				  << "\n✅ Configuration test successfully done.\n" 
				  << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED BOLD << "❌ Error : " << e.what() << RESET << std::endl;
		return 1;
	}

	return 0;
}
 */
#include "Webserv.hpp"

Socket *socket_a_close = NULL;
volatile sig_atomic_t	on = 1;

void	handle_sigint(int signum)
{
	(void)signum;
	if (socket_a_close)
		close(socket_a_close->getFd());
	on = 0;
}
int main(void)
{
	std::signal(SIGINT, handle_sigint);
	Server *ptr = NULL;
	Socket  *server = new Socket(std::string("127.0.0.1"), 8080, ptr);
	
	socket_a_close = server;
	std::vector<Socket *>	all_fd;
	
	all_fd.push_back(server);

	Monitor Moniteur(all_fd);
	Moniteur.Monitoring();
	for (std::vector<Socket *>::iterator it = all_fd.begin(); it < all_fd.end(); it++)
	{
		std::cout << "Suppression d'un socket server utiliser" << std::endl;
		delete(*it);
	}
}