#include "Webserv.hpp"

int	main(int ac, char **av)
{
	 std::string configFile;

	if (ac == 2)
		configFile = av[1];
	else if (ac == 1)
		configFile = "default/default.conf"; // default path
	else
	{
		std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
		return 1;
	}

	try
	{
		
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
