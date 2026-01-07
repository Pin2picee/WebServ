/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 20:34:42 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/07 16:18:10 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Monitor.hpp"

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
		tab_CGI = copy.tab_CGI;
		for (size_t i = 0; i < nb_fd; i++)
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
		tab_CGI = copy.tab_CGI;
		for (size_t i = 0; i < nb_fd; i++)
			all_fd[i] = copy.all_fd[i];
	}
	return (*this);
}

void   Monitor::add_fd(int &fd, int events)
{
	all_fd[this->nb_fd].fd = fd;
	all_fd[this->nb_fd].events = POLLIN | POLLHUP;
	if (events > 1)
		all_fd[this->nb_fd].events = POLLOUT;
	this->nb_fd++;
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
	Client  nouveau(all_socket.at(fd_server));
	uint32_t	ip_adress = ntohl(ip);
	uint16_t	port_adress = ntohs(port);
	std::ostringstream	oss;
	std::string ip_str;
	std::string port_str;
	unsigned char bytes[4];

	oss << port_adress;
	port_str = oss.str();//permet de le transformer en string
	bytes[0] = ip_adress >> 24 & 0xFF; 
	bytes[1] = ip_adress >> 16 & 0xFF;
	bytes[2] = ip_adress >> 8 & 0xFF;
	bytes[3] = ip_adress & 0xFF;

	oss.str("");//remet a zero le flux
	oss.clear();//reset flags aussi
	for (int i = 0; i < 4; i++)
	{
		oss << (int)bytes[i];//concatenation
		if (i < 3)
			oss << ".";
	}
	ip_str = oss.str();// ip en string grace a ostringstream .
	nouveau.setbasic(ip_str, port_str);
		clients.insert(std::pair<int, Client>(fd, nouveau));
}

int Monitor::deconnexion(int i)
{
    const int client_fd = all_fd[i].fd;
    std::map<int, Client>::iterator it_client = clients.find(client_fd);
    Client *client_ptr = (it_client != clients.end()) ? &it_client->second : 0;

    if (client_ptr)
        client_ptr->disconnected();
    close(client_fd);
    for (std::map<int, Client *>::iterator it = tab_CGI.begin(); it != tab_CGI.end(); )
    {
        std::map<int, Client *>::iterator cur = it++;
        if (cur->second == client_ptr)
            tab_CGI.erase(cur);
    }
    if (client_ptr && client_ptr->getCgiPid() > 0)
    {
        kill(client_ptr->getCgiPid(), SIGKILL);
        waitpid(client_ptr->getCgiPid(), NULL, WNOHANG);
    }
    int pipe_in  = client_ptr ? client_ptr->getPipeIn()  : -1;
    int pipe_out = client_ptr ? client_ptr->getPipeOut() : -1;
    for (size_t j = 0; j < nb_fd; )
    {
        int fdj = all_fd[j].fd;
        if (fdj == pipe_in || fdj == pipe_out)
            remove_fd(j);
        else
            ++j;
    }
    clients.erase(client_fd);
    all_fd[i] = all_fd[nb_fd - 1];
    all_fd[nb_fd - 1].fd = -1;
    nb_fd--;
    return 0;
}

int	 Monitor::test_read(ssize_t count)
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
		count = recv(all_fd[i].fd, buf, sizeof(buf), 0);
		result = test_read(count);
		if (!result)//client deco
			return (deconnexion(i));
		else if (result == 2)
			break;
		if (count > 0)
			buf_final.append(buf, count);
	}
	std::map<int, Client>::iterator it = clients.find(all_fd[i].fd);
	if (it != clients.end())
		it->second.setRequest(buf_final);
	return (1);
}

void Monitor::remove_fd(int index)
{
    if (all_fd[index].fd > 0)
	{
		close(all_fd[index].fd);
    	all_fd[index] = all_fd[nb_fd - 1];
    	all_fd[nb_fd - 1].fd = -1;
    	nb_fd--;
	}
}

int	Monitor::new_clients(int i)
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

void	Monitor::Timeout()
{
	timeval	timeofday;
	gettimeofday(&timeofday, NULL);
	long	time_sec = timeofday.tv_sec;
	long	time_usec = timeofday.tv_usec;
	std::vector<int>	Key_Erase;
	for (std::map<const int, Client *>::iterator it_tab_cgi = tab_CGI.begin(); it_tab_cgi != tab_CGI.end(); ++it_tab_cgi)
	{
		int	PipeIn = it_tab_cgi->second->getPipeIn();
		int	PipeOut = it_tab_cgi->second->getPipeOut();
		double resultat = (time_sec - it_tab_cgi->second->getCgiStartTime().tv_sec) + ((time_usec - it_tab_cgi->second->getCgiStartTime().tv_usec) / 1000000.0);
		if (resultat > 2.0)
		{
			std::cout << "TIMEOUT" << std::endl;
			kill(it_tab_cgi->second->getCgiPid(), SIGKILL);
			Response new_response;
			makeResponse(new_response, 504, readFile("config/www/errors/504.html"), "");
			it_tab_cgi->second->setReponse(it_tab_cgi->second->handler.responseToString(new_response));
			it_tab_cgi->second->setResponseGenerate(true);
			it_tab_cgi->second->setOutCGI();
			it_tab_cgi->second->setPipeAddPoll(false);
			waitpid(it_tab_cgi->second->getCgiPid(), NULL, WNOHANG);
			int	client_socket_fd = -1;
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (&(it->second) == it_tab_cgi->second)  // Comparer les adresses
				{
					client_socket_fd = it->first;  // ← La clé = fd du socket !
					break;
				}
			}
			for (size_t j = 0; j < nb_fd;)
			{
				if (all_fd[j].fd == client_socket_fd)
				{
					all_fd[j].events |= POLLOUT;
					break;
				}
				if (all_fd[j].fd == PipeOut || all_fd[j].fd == PipeIn)
					remove_fd(j);//->remove le fd des pipes du tab poll
				else
					j++;//->remove le fd des pipes du tab poll
			}
			if (tab_CGI.find(PipeIn) != tab_CGI.end())
				Key_Erase.push_back(PipeIn);
			if (tab_CGI.find(PipeOut) != tab_CGI.end())
				Key_Erase.push_back(PipeOut);
			it_tab_cgi->second->resetAfterCGI();
		}
	}
	for (std::vector<int>::iterator it_erase_key = Key_Erase.begin(); it_erase_key != Key_Erase.end(); it_erase_key++)
		tab_CGI.erase(*it_erase_key);
	Key_Erase.clear();
}

void	Monitor::remove_fd_CGI(Client *my_client)
{
	int PipeIn = my_client->getPipeIn();
	int PipeOut = my_client->getPipeOut();
	for (size_t i = 0; i < nb_fd;)
	{
		if (all_fd[i].fd == PipeOut || all_fd[i].fd == PipeIn)
			remove_fd(i);//->remove le fd des pipes du tab poll
		else
			i++;//->remove le fd des pipes du tab poll
	}
}

int	Monitor::pollout_CGI(int i, Client *my_client)
{
	if (all_fd[i].revents & POLLOUT && all_fd[i].fd == my_client->getPipeOut() && my_client->getBody() != "")
	{
		const std::string& body = my_client->getBody();
		int	reste = body.size() - my_client->getOffsetBodyCgi(); 
		std::cout << "Voici le body : " << body << "et voici ce qu'il reste a ecrire : " << reste << std::endl;
		if (reste > 0)
		{
			int	nb_written = write(all_fd[i].fd, body.c_str(), body.size());
			if (nb_written > 0)
			{
				my_client->AddOffsetBodyCgi(nb_written);
				return (-1);
			}
			if (nb_written == 0)
			{
				kill(my_client->getCgiPid(), SIGKILL);
				waitpid(my_client->getCgiPid(), NULL, WNOHANG);
				my_client->resetAfterCGI();
				remove_fd_CGI(my_client);
				return (-1);
			}
			if (nb_written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
				return (-1);
			else
			{
				kill(my_client->getCgiPid(), SIGKILL);
				waitpid(my_client->getCgiPid(), NULL, WNOHANG);
				std::cerr << "Erreur ecriture pipe CGI: " << strerror(errno) << std::endl;
				tab_CGI.erase(all_fd[i].fd);
				remove_fd_CGI(my_client);
				my_client->setOutCGI();
				i++;
				return (-1);
			}
		}
	}
	return (0);
}

int	Monitor::pollin_CGI(int i, Client *my_client)
{
	if ((all_fd[i].revents & POLLIN || all_fd[i].revents & POLLHUP) && all_fd[i].fd == my_client->getPipeIn())
	{
		char buffer[4096];
		ssize_t nb_read = -1;
		if (all_fd[i].fd > 0)
			nb_read = read(all_fd[i].fd, &buffer, sizeof(buffer));
		if (nb_read > 0)
		{
			my_client->AddCgiOutput(std::string(buffer, nb_read));
			nb_read = 0;
			return (-1);
		}
		if (nb_read == 0 || (nb_read < 0 && all_fd[i].revents & POLLHUP))
		{
			std::cout << "EGAL 0" << std::endl;
			Response new_response = parseCGIOutput(my_client->getCgiOutput());
			my_client->setReponse(my_client->handler.responseToString(new_response));
			my_client->setResponseGenerate(true);
			my_client->setOutCGI();
			waitpid(my_client->getCgiPid(), NULL, WNOHANG);							
			int client_socket_fd = -1;
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (&(it->second) == my_client)  // Comparer les adresses
				{
					client_socket_fd = it->first;  // ← La clé = fd du socket !
					break;
				}
			}
			for (size_t j = 0; j < nb_fd; j++)
			{
				if (all_fd[j].fd == client_socket_fd)
				{
					all_fd[j].events |= POLLOUT;
					break;
				}
			}
			std::map<int, Client*>::iterator it_temp = tab_CGI.find(all_fd[i].fd);  
			if (it_temp != tab_CGI.end())
			{
				if (tab_CGI.find(it_temp->second->getPipeOut()) != tab_CGI.end())
					tab_CGI.erase(it_temp->second->getPipeOut());
				tab_CGI.erase(all_fd[i].fd);
			}
			remove_fd_CGI(my_client);
			my_client->resetAfterCGI();
			return (-1);
		}
		if (nb_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
			return (-1);
		else
		{
			std::cout << "else" << std::endl;
			kill(my_client->getCgiPid(), SIGKILL);
			waitpid(my_client->getCgiPid(), NULL, WNOHANG);
			Response new_response;
			makeResponse(new_response, 504, readFile("config/www/errors/504.html"), "");
			my_client->setReponse(my_client->handler.responseToString(new_response));
			my_client->setResponseGenerate(true);
			my_client->setOutCGI();
			my_client->setPipeAddPoll(false);
			my_client->resetAfterCGI();
			int	fd_current = -1;
			for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				if (&(it->second) == my_client)  // Comparer les adresses
				{
					fd_current = it->first;  // ← La clé = fd du socket !
					break;
				}
			}
			for (size_t j = 0; j < nb_fd; j++)
			{
				if (all_fd[j].fd == fd_current)
				{
					all_fd[j].events |= POLLOUT;
					break;
				}
			}
			std::map<int, Client*>::iterator it_temp = tab_CGI.find(all_fd[i].fd);  
			if (it_temp != tab_CGI.end())
			{
				if (tab_CGI.find(it_temp->second->getPipeOut()) != tab_CGI.end())
					tab_CGI.erase(it_temp->second->getPipeOut());
				tab_CGI.erase(all_fd[i].fd);
			}
			remove_fd_CGI(my_client);
			return (-1);
		}
	}
	return (0);
}

int	Monitor::CGI_engine(int i)
{
	if (tab_CGI.find(all_fd[i].fd) != tab_CGI.end())
	{
		Client *my_client = tab_CGI[all_fd[i].fd];
		if (pollout_CGI(i, my_client) < 0)
			return (-1);
		if (pollin_CGI(i, my_client) < 0)
			return(-1);
	}
	return (0);
}

/**
 * @brief = Une boucle poll qui verifie chaque socket server et qui accept les connexions
 */
void	Monitor::Monitoring()
{
	int poll_return;

	std::cout << "Lancement du server" << std::endl;
	int	poll_reveil = 0;
	
	while (on)
	{
		poll_return = poll(this->all_fd, nb_fd, 15);
		Timeout();
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
			for (size_t i = 0; i < nb_fd;)//parcours les socket
			{
				std::map<int, Client>::iterator it_client = clients.find(all_fd[i].fd);
				bool client_disconnected = false;
				if (CGI_engine(i) < 0)
					continue;
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
					if (i < nb_fd_server)
					{
						new_clients(i++);
						continue;
					}
					else if (i >= nb_fd_server && !new_request(i))//client/deconnexion
						continue;// on reviens sur le meme i car il a etait changer par le dernier dans  deconnexion
				}
				if (i >= this->nb_fd_server && all_fd[i].fd > 0 && it_client != clients.end() && it_client->second.getFinishRequest())
					all_fd[i].events |= POLLOUT;
				if (all_fd[i].revents & POLLOUT && i >= nb_fd_server && it_client != clients.end())
				{
					int	  nb_send = 0;
					size_t		offset;

					offset = it_client->second.getOffset();
					if ((it_client->second.getSyntax() || it_client->second.getFinishRequest()) && offset == 0 && it_client->second.getInCGI() == false && !it_client->second.getResponseGenerate())
					{
						Request request = it_client->second.ExtractRequest();
						Response	structResponse = it_client->second.handler.handleRequest(request, &it_client->second);
						if (it_client->second.getInCGI())
							it_client->second.setResponseGenerate(true);
						else
						{
							it_client->second.setReponse(it_client->second.handler.responseToString(structResponse));
							it_client->second.setResponseGenerate(true);
						}
					}
					if (it_client->second.getInCGI() == true && !it_client->second.getPipeAddPoll())
					{
						int	PipeIn = it_client->second.getPipeIn();
						int	PipeOut = it_client->second.getPipeOut();
						if (PipeIn > 0)
						{
							int	flags = fcntl(PipeIn, F_GETFL);
							fcntl(PipeIn, F_SETFL, flags | O_NONBLOCK);
							add_fd(PipeIn, 1);
							tab_CGI.insert(std::make_pair(PipeIn, &(it_client->second)));
						}
						if (PipeOut > 0)
						{
							int	flags = fcntl(PipeOut, F_GETFL);
							fcntl(PipeOut, F_SETFL, flags | O_NONBLOCK);
							add_fd(PipeOut, 2);
							tab_CGI.insert(std::make_pair(PipeOut, &(it_client->second)));
						}
						it_client->second.setPipeAddPoll(true);
					}
					if (it_client->second.getReponse() != "")
						nb_send = send(all_fd[i].fd,  it_client->second.getReponse().c_str() + offset, it_client->second.getReponse().length() - offset, 0);
					if (nb_send < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
					{
						perror("ERROR : SEND FAILED\n");
						std::cout << all_fd[i].fd << std::endl;
					}	
					if (nb_send > 0)
						it_client->second.AddOffset(nb_send);
					if (it_client->second.getOffset() >= it_client->second.getReponse().length() && !it_client->second.getInCGI() 
						&& it_client->second.getReponse().length() > 0)
					{
						all_fd[i].events = POLLIN;
						it_client->second.setReponse("");
						it_client->second.setResponseGenerate(false);
						it_client->second.resetAfterCGI();
						it_client->second.setOutCGI();
						it_client->second.ResetCgiOutput();
					}
				}
				i++;
			}
		}
	}
	clean_CGI();
}

void	Monitor::clean_CGI()
{
		for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.getInCGI())
		{
			if (it->second.getPipeIn() > 0)
			{
				close(it->second.getPipeIn());
				it->second.setPipeIn(-1);
			}
			if (it->second.getPipeOut() > 0)
			{
				close(it->second.getPipeOut());
				it->second.setPipeOut(-1);
			}
			pid_t pid = it->second.getCgiPid();
			kill(pid, SIGKILL);  // Essayer un arrêt propre
			usleep(100000);      // Attendre 100ms
			kill(pid, SIGKILL);  // Forcer si nécessaire
			waitpid(pid, NULL, WNOHANG);
		}
	}
}