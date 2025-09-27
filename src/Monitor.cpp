/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:12:02 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/27 17:10:37 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Monitor.hpp"

/*<Exception>*/
const char *Monitor::MonitorError::what() const throw()
{
	return (strerror(errno));
}

/*</Exception>*/

/*<Construction>*/

Monitor::Monitor() {}

Monitor::~Monitor() {}

Monitor::Monitor(const Monitor &copy)
{
	if (this != &copy)
	{
		nb_socket = copy.nb_socket;
		nb_socket_server = copy.nb_socket_server;
		clients = copy.clients;
		all_socket[5000] = copy.all_socket[5000];
	}
}

Monitor &Monitor::operator=(const Monitor &copy)
{
	if (this != &copy)
	{
		nb_socket = copy.nb_socket;
		nb_socket_server = copy.nb_socket_server;
		clients = copy.clients;
		all_socket[5000] = copy.all_socket[5000];
	}
	return (*this);
}

void   Monitor::add_fd(int &fd)
{
	all_socket[this->nb_socket].fd = fd; 
	all_socket[this->nb_socket].events = POLLIN;
	this->nb_socket++;
	this->nb_socket_server;
}

Monitor::Monitor(std::vector<int> tab)
{
	this->nb_socket = 0;
	while (nb_socket < tab.size())
	{
		all_socket[nb_socket].fd = tab[nb_socket]; 
		all_socket[nb_socket].events = POLLIN;
		nb_socket++;
	}
	nb_socket_server = nb_socket;
}

/*</Construction>*/

/*AJOUT DU CLIENT DANS LA MAP FD:CLIENT*/
void Monitor::add_client(int fd, in_addr_t ip, in_port_t port)
{
	Client  nouveau;
	uint32_t    ip_adress = ntohl(ip);
	uint16_t    port_adress = ntohs(port);
	std::string ip_str;
	std::string port_str;
	unsigned char bytes[4];

	port_str = std::to_string(port_adress);
	bytes[0] = ip_adress >> 24 & 0xFF; 
	bytes[1] = ip_adress >> 16 & 0xFF;
	bytes[2] = ip_adress >> 8 & 0xFF;
	bytes[3] = ip_adress & 0xFF;

	for (int i = 0; i < 4; i++)
	{
		ip_str += std::to_string(bytes[i]);
		if (i < 3)
			ip_str += '.';
	}
	nouveau.setbasic(ip_str, port_str);
	clients[fd] = nouveau;
}

int     Monitor::deconnexion(int i)
{
	/**
	 * deconnexion cote map client + affichage du log
	 * fermeture du socket
	 * remplacement par le dernier
	 * actualisation du nb de socket
	 */
	clients[all_socket[i].fd].deconected();
	close(all_socket[i].fd);
	all_socket[i] = all_socket[nb_socket - 1];
	all_socket[nb_socket - 1].fd = -1;//poll ne le surveillera plus
	nb_socket--;
	return (0);
}

int     Monitor::test_read(int i, ssize_t count)
{
	if (count > 0)
		return (1);
	else if (count < 0)//rien a lire pour l'instant ou deco
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)// r a lire
			return (0);
		else// alors deconnexion
			return (deconnexion(i));
	}
	else//(count == 0)//deco
		return (deconnexion(i));
}

int		Monitor::new_request(int i)
{
	ssize_t count;
	char buf[1024];
	std::string buf_final;
	
	while (42)
	{
		count = read(all_socket[i].fd, buf, sizeof(buf));
		if (!test_read(i, count))//client deco
			break;
		buf_final.append(buf, count);
	}
	clients[all_socket[i].fd].setRequest(buf_final);
	return (0);
}

int	Monitor::new_clients(int i)
{
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	all_socket[nb_socket].fd = accept(all_socket[i].fd, (sockaddr *)&address, &addrlen);
	all_socket[nb_socket].events = POLLIN | POLLOUT;
	add_client(all_socket[nb_socket].fd, address.sin_addr.s_addr, address.sin_port);// on l'ajoute dans ma map de clients
	if (all_socket[nb_socket].fd < 0)
		return (std::cout << "ACCEPTE ECHOUE"<< std::endl, 1) ;
		
	int ancien_flags = fcntl (all_socket[nb_socket].fd, F_GETFL);
	fcntl(all_socket[nb_socket].fd, F_SETFL, ancien_flags | O_NONBLOCK);
	nb_socket++;
	return (0);
}

/**
 * @brief = Une boucle poll qui verifie chaque socket server et qui accept les connexions
 */

void    Monitor::Monitoring()
{
	int poll_return;

	while (42)
	{
		poll_return = poll(this->all_socket, nb_socket, 15);
		if (poll_return == 0)//AUCUN SOCKET du TAB n'est pret timeout
			continue ;
		else if (poll_return < 0)//ERROR
			throw MonitorError();
		else if (poll_return > 0)//un ou plusieurs socket pret
		{
			for (int i = 0; i < nb_socket; i++)//parcours les socket
			{
				if (all_socket[i].revents & POLLIN)// read ready
				{
					if (i < nb_socket_server && new_clients(i))// nouvelle connexion client a accept
							continue;
					else if (i >= nb_socket_server && new_request(i))//client
							perror("Error : Accept()\n");
				}
				if (all_socket[i].revents & POLLOUT && i >= nb_socket_server)//cote client je peux ecrire
				{
					std::string reponse;
					size_t      nb_send;
					
					reponse =  clients[all_socket[i].fd].getReponse();
					nb_send = write(all_socket[i].fd, reponse.c_str(), reponse.length());
					if (nb_send <= 0)
						perror("ERROR : SEND FAILED \n");
				}
			}
		}
	}
	
}
