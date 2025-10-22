/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:12:02 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/15 14:57:23 by abelmoha         ###   ########.fr       */
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
		nb_fd = copy.nb_fd;
		nb_fd_server = copy.nb_fd_server;
		clients = copy.clients;
		for (int i = 0; i < nb_fd; i++)
			all_fd[i] = copy.all_fd[i];
	}
}

Monitor &Monitor::operator=(const Monitor &copy)
{
	if (this != &copy)
	{
		nb_fd = copy.nb_fd;
		nb_fd_server = copy.nb_fd_server;
		clients = copy.clients;
		all_fd[5000] = copy.all_fd[5000];
	}
	return (*this);
}

void   Monitor::add_fd(int &fd)
{
	all_fd[this->nb_fd].fd = fd; 
	all_fd[this->nb_fd].events = POLLIN;
	this->nb_fd++;
	this->nb_fd_server++;
}

Monitor::Monitor(std::vector<Socket *> tab)
{
	this->nb_fd = 0;
	std::vector<Socket *>::iterator	it = tab.begin();
	for (size_t i = nb_fd; i < 200000; i++)
    {
        all_fd[i].fd = -1;
        all_fd[i].events = 0;
        all_fd[i].revents = 0;
    }
	while (nb_fd < tab.size() && it != tab.end())
	{
		all_fd[nb_fd].fd = (*it)->getFd();
		all_fd[nb_fd].events = POLLIN;
		all_socket.insert(std::make_pair((*it)->getFd(), *it));
		nb_fd++;
		it++;
	}
	nb_fd_server = nb_fd;
}

/*</Construction>*/

/*AJOUT DU CLIENT DANS LA MAP FD:CLIENT*/
void Monitor::add_client(int fd, in_addr_t ip, in_port_t port, int fd_server)
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
	nouveau.set_socket(all_socket.at(fd_server));
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
	clients[all_fd[i].fd].deconected();
	close(all_fd[i].fd);
	clients.erase(all_fd[i].fd);
	all_fd[i] = all_fd[nb_fd - 1];
	all_fd[nb_fd - 1].fd = -1;//poll ne le surveillera plus
	nb_fd--;
	return (0);
}

int     Monitor::test_read(ssize_t count)
{
	if (count > 0)
		return (1);
	else if (count < 0)//rien a lire pour l'instant ou deco
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)// r a lire
			return (2);
		else// alors deconnexion
			return (0);
	}
	else//(count == 0)//deco
		return (0);
}

int		Monitor::new_request(int i)
{
	ssize_t count;
	char buf[1024];
	std::string buf_final;
	int	result;
	
	while (42)
	{
		count = read(all_fd[i].fd, buf, sizeof(buf));
		result = test_read(count);
		if (!result)//client deco
			return (deconnexion(i));
		else if (result == 2)
			break;
		if (count > 0)
			buf_final.append(buf, count);
	}
	clients[all_fd[i].fd].setRequest(buf_final);
	return (1);
}

int	Monitor::	new_clients(int i)
{
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	all_fd[nb_fd].fd = accept(all_fd[i].fd, (sockaddr *)&address, &addrlen);
	all_fd[nb_fd].events = POLLIN;
	if (all_fd[nb_fd].fd < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return (std::cout << "RIEN A ACCEPTER"<< std::endl, 1) ;
	else if (all_fd[nb_fd].fd < 0)
		return (std::cout << "ERROR ressources accept" << std::endl, 1);
	add_client(all_fd[nb_fd].fd, address.sin_addr.s_addr, address.sin_port, all_fd[i].fd);// on l'ajoute dans ma map de clients avec le fd du server
	
	int ancien_flags = fcntl (all_fd[nb_fd].fd, F_GETFL);
	fcntl(all_fd[nb_fd].fd, F_SETFL, ancien_flags | O_NONBLOCK);
	nb_fd++;
	return (0);
}

/**
 * @brief = Une boucle poll qui verifie chaque socket server et qui accept les connexions
 */

void    Monitor::Monitoring()
{
	int poll_return;

	std::cout << "Lancement du server" << std::endl;
	int	poll_reveil = 0;
	while (on)
	{
		
		poll_return = poll(this->all_fd, nb_fd, 15);
		if (poll_return == 0)//AUCUN SOCKET du TAB n'est pret timeout
			continue ;
		else if (poll_return < 0)//ERROR
		{
			if (errno == EINTR)//signal recu donc erreur poll
				break;
			throw MonitorError();
		}
		else if (poll_return > 0)//un ou plusieurs socket pret
		{
			poll_reveil++;
			for (int i = 0; i < nb_fd;)//parcours les socket
			{
				bool client_disconnected = false;
				//deja deconnecte en general le dernier car deconnexion switch
				if (all_fd[i].fd < 0 || all_fd[i].revents & POLLNVAL)
				{
					i++;
					continue;
				}
				//ERROR socket
				if (all_fd[i].revents & POLLERR)
				{
					if (i >= nb_fd_server)
					{
						deconnexion(i);
						continue;
					}
					else
					{
						std::cout << "Problem socket server" << std::endl;
						i++;
					}
				}
				//deconnexion du client
				if (all_fd[i].revents & POLLHUP)
				{
					if (i >= nb_fd_server)
					{
						if (all_fd[i].revents & POLLIN)
							client_disconnected = new_request(i);//lit les derniers donne
						if (client_disconnected)
							deconnexion(i);//on deconnecte + affiche le log//met a jour i
						continue;
					}
					else
					{
						i++;
						continue;
					}
				}
				//lecture
				if (all_fd[i].revents & POLLIN)
				{
					std::cout << ".";
					if (i < nb_fd_server)
					{
						new_clients(i++);
						continue;
					}
					else if (i >= nb_fd_server && !new_request(i))//client/deconnexion
							continue;// on reviens sur le meme i car il a etait changer par le dernier dans  deconnexion
				}
				//ecriture
				if (clients[all_fd[i].fd].getFinishRequest())
					all_fd[i].events |= POLLOUT;
				if (all_fd[i].revents & POLLOUT && i >= nb_fd_server)//cote client je peux ecrire
				{
					std::string reponse;
					size_t      nb_send;
					size_t		offset;
					
					//objet REQUEST_HANDLER qui fait appel a l'objet ParsingHttp et SendHttp
					/**
					 * Mon request Handler fait appel a parsingHTTP si pas bon request handler requete false
					 * si bon savoir si POST, delete, GET
					 * SENDhttp s'occupe de generer la requete reponse
					 */
					reponse = clients[all_fd[i].fd].getReponse();
					offset = clients[all_fd[i].fd].getOffset();
					nb_send = write(all_fd[i].fd, reponse.c_str() + offset, reponse.length() - offset);
					if (nb_send < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
						perror("ERROR : SEND FAILED \n");
					if (nb_send > 0)
						clients[all_fd[i].fd].AddOffset(nb_send);
					if (clients[all_fd[i].fd].getOffset() >= reponse.length())
						all_fd[i].events = POLLIN;
				}
				i++;
			}
		}
	}
	
}
