#include "Webserv.hpp"

int main(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << RED BOLD << "Usage : ./webserv [configuration file]" RESET << std::endl;
		return 1;
	}

	std::string configFile = (ac == 2) ? av[1] : "config/default.conf";

	try
	{
		std::cout << CYAN BOLD << "📄 Chargement du fichier de configuration : " 
				  << configFile << RESET << std::endl;

		Config parser;
		parser.parseAllServerFiles(configFile);

		const std::vector<Server> &servers = parser.getServers();

		std::cout << GREEN BOLD << "\n🌐 Nombre de serveurs configurés : " 
				  << servers.size() << RESET << std::endl;

		// --- Affichage du contenu de chaque serveur ---
		for (size_t s = 0; s < servers.size(); ++s)
		{
			const Server &serv = servers[s];
			std::cout << MAGENTA BOLD << "\n==============================" << RESET << std::endl;
			std::cout << YELLOW BOLD << "🖥️  Serveur #" << s + 1 << RESET << std::endl;

			// ROOT
			std::cout << BOLD BLUE "Root: " RESET << serv.getRoot() << std::endl;

			// LISTEN
			std::cout << BOLD GREEN "Listen: " RESET;
			for (size_t j = 0; j < serv.getListen().size(); ++j)
			{
				std::cout << serv.getListen()[j].first << ":" 
						  << serv.getListen()[j].second << RESET;
				if (j + 1 < serv.getListen().size())
					std::cout << ", ";
			}
			std::cout << std::endl;

			// MAX BODY SIZE
			std::cout << BOLD YELLOW "Max body size: " RESET 
					  << serv.getClientMaxBodySize() << std::endl;

			// ERROR PAGES
			std::cout << BOLD RED "Error pages:" RESET << std::endl;
			for (std::map<int, std::string>::const_iterator it = serv.getErrorPages().begin();
				 it != serv.getErrorPages().end(); ++it)
			{
				std::cout << "  " << it->first << " => " << it->second << std::endl;
			}
			std::cout << std::endl;

			// LOCATIONS
			std::cout << BOLD MAGENTA "Locations: " RESET << serv.getLocations().size() << std::endl;
			for (size_t l = 0; l < serv.getLocations().size(); ++l)
			{
				const Locations &loc = serv.getLocations()[l];
				std::cout << "  " CYAN "- Location #" << l + 1 << RESET << std::endl;
				std::cout << "    " BOLD "Path: " RESET << BLUE << loc.path << RESET << std::endl;
				std::cout << "    " BOLD "Root: " RESET << BLUE << loc.root << RESET << std::endl;
				std::cout << "    " BOLD "Autoindex: " RESET 
						  << (loc.autoindex ? GREEN "on" RESET : RED "off" RESET) 
						  << std::endl;
			}
		}

		std::cout << GREEN BOLD 
				  << "\n✅ Test de configuration terminé avec succès.\n" 
				  << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << RED BOLD << "❌ Erreur : " << e.what() << RESET << std::endl;
		return 1;
	}

	return 0;
}
