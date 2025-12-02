#include "Webserv.hpp"

int main (int argc, char **argv)
{
	std::string ConfigFileName;
	signal(SIGINT, handle_sigint);
	if (argc > 2)
	{
		std::cerr << RED BOLD << "Usage : ./webserv [configuration file]" RESET << std::endl;
		return 1;
	}
	ConfigFileName = (argc == 2) ? argv[1] : "config/default.conf";
	try
	{
		Config parser;
		parser.parseAllServerFiles(ConfigFileName);
		all_socket = parser.getSocket();
		Monitor	Moniteur(all_socket);
		Moniteur.Monitoring();
	}
	catch (std::exception &e)
	{
		std::cout << "ERROR:" << e.what() << std::endl;
	}
	return (0);
}
