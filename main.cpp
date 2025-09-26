#include "Webserv.hpp"
#include "ConfigParser.hpp"

int main(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << "Usage : ./webserv [configuration file]\n";
		return 1;
	}

	std::string configFile = (ac == 2) ? av[1] : "config/default.conf";

	try
	{
		ConfigParser parser(configFile);
		ServerConf conf = parser.parse();

		std::cout << "Config loaded. First listen : "
				  << conf.listen[0].first << " : " << conf.listen[0].second
				  << std::endl;
	}
	catch (const std::exception &e) {
		std::cerr << "Error : " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
