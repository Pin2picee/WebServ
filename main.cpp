/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <locagnio@student.42perpignan.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/07 20:31:51 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/16 03:59:17 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

/**
 * @brief
 * Entry point for the Webserv project.
 *
 * Initializes the server by parsing the configuration file, setting up signals,
 * creating and monitoring server sockets, and cleaning up resources on exit.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 */
/**
 *             - If no argument is provided, uses "config/default.conf".
 */
/**
 *             - If one argument is provided, uses it as the configuration file path.
 *
 * @return Returns 0 on successful execution, 1 if usage is incorrect.
 */
int main(int argc, char **argv)
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
		signal(SIGPIPE, SIG_IGN);
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
		std::cerr << RED BOLD << "Error: " << e.what() << RESET << std::endl;
	}
	return (0);
}
