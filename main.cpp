#include "Webserv.hpp"
#include "ConfigParser.hpp"
#include "utils.hpp"

int main(int ac, char **av)
{
	if (ac > 2)
	{
		std::cerr << RED BOLD << "Usage: ./webserv [configuration file]" RESET << std::endl;
		return 1;
	}

	std::string configFile = (ac == 2) ? av[1] : "config/default.conf";

	try
	{
		ConfigParser parser(configFile);
		ServerConf conf = parser.parse();

		std::cout << BOLD CYAN << "===== Server Configuration =====" RESET << "\n";

		// Listen
		std::cout << BOLD YELLOW << "* Listen on :" RESET "\n";
		for (size_t i = 0; i < conf.listen.size(); ++i)
			std::cout << "  " << GREEN << conf.listen[i].first << RESET << CYAN " : " 
					  << YELLOW << conf.listen[i].second << RESET << "\n";

		// Root
		std::cout << BOLD YELLOW << "\n* Root : " RESET << MAGENTA << conf.root << RESET << "\n";

		// Client max body size
		std::cout << BOLD YELLOW << "\n* Client max body size : " RESET 
				  << MAGENTA << conf.client_max_body_size << CYAN " bytes\n" << RESET;

		// Error pages
		std::cout << BOLD YELLOW << "\n* Error pages :" RESET "\n";
		for (std::map<int, std::string>::const_iterator it = conf.error_pages.begin();
			 it != conf.error_pages.end(); ++it)
			std::cout << "  " << RED << it->first << RESET << CYAN " -> " 
					  << MAGENTA << it->second << RESET << "\n";

		// Locations
		std::cout << BOLD YELLOW << "\n* Locations :" RESET "\n";
		for (size_t i = 0; i < conf.locations.size(); ++i)
		{
			const LocationConf &loc = conf.locations[i];
			std::cout << YELLOW "- Path : " RESET << CYAN << loc.path << RESET << "\n";
			std::cout << "    " YELLOW "Root : " RESET << MAGENTA << loc.root << RESET << "\n";
			std::cout << "    " YELLOW "Upload dir : " RESET << MAGENTA << loc.upload_dir << RESET << "\n";
			std::cout << "    " YELLOW "CGI : " RESET << (loc.cgi ? GREEN "on" RESET : RED "off" RESET) << "\n";
			std::cout << "    " YELLOW "CGI extension : " RESET << MAGENTA << loc.cgi_extension << RESET << "\n";
			std::cout << "    " YELLOW "Autoindex : " RESET << (loc.autoindex ? GREEN "on" RESET : RED "off" RESET) << "\n";

			std::cout << "    " YELLOW "Methods : " RESET;
			if (loc.methods.empty())
				std::cout << RED "none" RESET;
			else
				for (size_t j = 0; j < loc.methods.size(); ++j)
					std::cout << GREEN << loc.methods[j] << RESET << " ";
			std::cout << "\n";

			std::cout << "    " YELLOW "Index files : " RESET;
			if (loc.index_files.empty())
				std::cout << RED "none" RESET;
			else
				for (size_t j = 0; j < loc.index_files.size(); ++j)
					std::cout << MAGENTA << loc.index_files[j] << RESET << " ";
			std::cout << "\n\n";
		}

		std::cout << BOLD CYAN << "================================" RESET << "\n";
		std::cout << BOLD MAGENTA << "===== Testing Handlers =====" RESET << "\n";

		// Créer une requête POST factice
		Request postReq;
		postReq.method = "POST";
		postReq.uri = "/upload/testfile.txt";
		postReq.body = "Hello Webserv!";
		postReq.headers["Content-Length"] = ft_to_string(postReq.body.size());

		// On prend la location /upload
		const LocationConf &uploadLoc = conf.locations[1]; // normalement /upload
		Response postResp = handlePost(uploadLoc, postReq, conf);

		std::cout << GREEN << "POST Response code: " << RESET << postResp.status_code << "\n";
		std::cout << GREEN << "POST Response body: " << RESET << postResp.body << "\n\n";

		// Créer une requête DELETE factice
		Request delReq;
		delReq.method = "DELETE";
		delReq.uri = "/upload/testfile.txt";

		Response delResp = handleDelete(uploadLoc, delReq);

		std::cout << GREEN << "DELETE Response code: " << RESET << delResp.status_code << "\n";
		std::cout << GREEN << "DELETE Response body: " << RESET << delResp.body << "\n\n";

	}
	catch (const std::exception &e)
	{
		std::cerr << RED BOLD << "Error: " << e.what() << RESET << std::endl;
		return 1;
	}

	return 0;
}