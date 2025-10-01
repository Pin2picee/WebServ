/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 10:48:18 by abelmoha@st       #+#    #+#             */
/*   Updated: 2025/10/01 21:24:48 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MONITOR_HPP

# define MONITOR_HPP

# include <vector>
# include <map>
# include <iostream>
# include <poll.h>
# include <errno.h>
# include <string.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include "Client.hpp"
# include <fcntl.h>

/**
 * @brief = FUTUR instance qui va permettre de poll mes socket serveurs et client car on a le droit a un seul poll
 * @example = si monitoring lancer on ne add_fd plus et on ne rajoute plus de socket server
 */

class Monitor
{
	private :
		size_t				nb_socket;
		size_t				nb_socket_server;
		std::map<int, Client>	clients;
		struct  pollfd		all_socket[20000];// dois tout avoir dans ce meme tab connexion for read & write
	private :
		Monitor();
		void	add_client(int fd, in_addr_t ip, in_port_t port);// ajoute un client  dans mon vecteur de client
		int		new_clients(int i);//ajoute un nouveau socket non bloquant pour le client
		int		new_request(int i);// RECUP la requete + verif test_read
		int		test_read(ssize_t count);
		int		deconnexion(int i);//SUPPRIME LA CONNEXION SOCKET CLIENT IF !TEST_READ
	public :
		Monitor(std::vector<int> tab);
		~Monitor();
		Monitor(const Monitor &copy);
		Monitor &operator=(const Monitor &copy);
	public :
		void	add_fd(int &fd);//ajoute un nouveaux socket dans le tab pour poll / a faire avant le monitoring
		void	Monitoring();//principal

	class MonitorError : public std::exception
	{
		const char *what() const throw();
	};
};

#endif