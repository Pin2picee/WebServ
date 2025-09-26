#include "Webserv.hpp"
#include "ConfigParser.hpp"

int main(int ac, char **av)
{
    std::string configFile;

    if (ac == 2)
        configFile = av[1];
    else if (ac == 1)
        configFile = "config/default.conf";
    else
	{
        std::cerr << "Usage : ./webserv [configuration file]" << std::endl;
        return 1;
    }

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
