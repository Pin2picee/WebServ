/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/07 20:31:51 by abelmoha          #+#    #+#             */
/*   Updated: 2025/12/16 21:45:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		/*
		Client	test();
		
		test.setRequest("GET /api/users HTTP/1.1\r\nHost: localhost \r\nUser-Agent:\r\n\r\n");
		Request nv = test.ExtractRequest();
		std::cout << "Methode :" << nv.method << std::endl;
		std::cout << "uri :" << nv.uri << std::endl;
		std::cout << "path :" << nv.path << std::endl;
		std::cout << "Version :" << nv.version << std::endl;
		for (std::map<std::string, std::string>::iterator it = nv.headers.begin(); it != nv.headers.end(); it++ )
		std::cout << "HEADERS :" << it->first << ":" << it->second << std::endl;
		std::cout << "body :" << nv.body << std::endl;
		*/
	}
	catch (std::exception &e)
	{
		std::cout << "ERROR:" << e.what() << std::endl;
	}
	return (0);
}