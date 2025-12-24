/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/07 20:31:51 by abelmoha          #+#    #+#             */
/*   Updated: 2025/12/24 01:42:06 by abelmoha         ###   ########.fr       */
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
		for (std::vector<Socket*>::iterator it = all_socket.begin(); it != all_socket.end(); ++it)
    	{
			if (*it)
			{
				close((*it)->getFd());
				delete *it;
			}
    	}
	}
	catch (std::exception &e)
	{
		std::cout << "ERROssR:" << e.what() << std::endl;
	}
	return (0);
}