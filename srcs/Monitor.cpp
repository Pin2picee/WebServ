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

int	 Monitor::deconnexion(int i)
{
	/**
	 * deconnexion cote map client + affichage du log
	 * fermeture du socket
	 * remplacement par le dernier
	 * actualisation du nb de socket
	 */
	std::map<int, Client>::iterator it = clients.find(all_fd[i].fd);
	if (it != clients.end())
	it->second.disconnected();
	close(all_fd[i].fd);
	clients.erase(all_fd[i].fd);
	all_fd[i] = all_fd[nb_fd - 1];
	all_fd[nb_fd - 1].fd = -1;//poll ne le surveillera plus
	nb_fd--;
	return (0);
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

void	Monitor::Monitoring()
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
			for (size_t i = 0; i < nb_fd;)//parcours les socket
			{
				std::map<int, Client>::iterator it_client = clients.find(all_fd[i].fd);
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
					if (i < nb_fd_server)
					{
						new_clients(i++);
						continue;
					}
					else if (i >= nb_fd_server && !new_request(i))//client/deconnexion
						continue;// on reviens sur le meme i car il a etait changer par le dernier dans  deconnexion
				}
				//ecriture
				if (i >= this->nb_fd_server && all_fd[i].fd > 0 && it_client != clients.end() && it_client->second.getFinishRequest())
					all_fd[i].events |= POLLOUT;
				if (all_fd[i].revents & POLLOUT && i >= nb_fd_server && it_client != clients.end())//cote client je peux ecrire
				{
					int	  nb_send;
					size_t		offset;

					offset = it_client->second.getOffset();// le nombre de caracteres envoyer: garder en memoire
					
					if ((it_client->second.getSyntax() || it_client->second.getFinishRequest()) && offset == 0)
					{
						
						//Il faudrait avoir la struct request directement dans le corp du client puis extraire et parser la request seulement si setrequest a fini et que le booleen est = true
						/*
						
						std::cout << "Method :" << request.method << std::endl;
						std::cout << "uri :" << request.uri << std::endl;
						std::cout << "path :" << request.path << std::endl;
						std::cout << "version :" << request.version << std::endl;
						std::cout << RED << "Headers : " << RESET << std::endl;
						
						for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); it++)
						{
							std::cout << it->first << ":" << it->second << std::endl;
						}
						*/
						Request request = it_client->second.ExtractRequest();
						Response	structResponse = it_client->second.handler.handleRequest(request);
						displayResponseInfo(structResponse);
						updateClientCookies(it_client->second, structResponse);
						it_client->second.setReponse(it_client->second.handler.responseToString(structResponse)); 
					}
					nb_send = send(all_fd[i].fd,  it_client->second.getReponse().c_str() + offset, it_client->second.getReponse().length() - offset, 0);
					if (nb_send < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
						perror("ERROR : SEND FAILED \n");
					if (nb_send > 0)
						it_client->second.AddOffset(nb_send);
					if (it_client->second.getOffset() >= it_client->second.getReponse().length())
						all_fd[i].events = POLLIN;
						
				}
				i++;
			}
		}
	}
	
}

static void parseSetCookie(const std::string &header, std::string &name, std::string &value)
{
    size_t start = header.find("Set-Cookie: ");
    if (start == std::string::npos)
        return;

    start += 12; // longueur de "Set-Cookie: "
    size_t end = header.find(';', start);
    std::string cookie_pair = header.substr(start, end - start);

    size_t eq = cookie_pair.find('=');
    if (eq != std::string::npos)
    {
        name = cookie_pair.substr(0, eq);
        value = cookie_pair.substr(eq + 1);
    }
}

void Monitor::updateClientCookies(Client &client, const Response &resp)
{
    std::map<std::string, std::string> &clientCookies = client.getCookies();
    if (!clientCookies.empty())
        return;
    for (std::vector<std::string>::const_iterator it = resp.headers.begin(); it != resp.headers.end(); ++it)
    {
        std::string name, value;
        parseSetCookie(*it, name, value);
        if (!name.empty() && !value.empty())
            client.setCookies(name, value);
    }
}

