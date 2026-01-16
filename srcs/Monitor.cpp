/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <locagnio@student.42perpignan.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 20:34:42 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/16 04:36:51 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Monitor.hpp"

/**
 * @brief
 * Custom exception for `Monitor` that returns the system `errno` message.
 *
 * @return Returns the system error message.
 */
const char *Monitor::MonitorError::what() const throw()
{
	return (strerror(errno));
}

/**
 * @brief
 * Checks if a `Client` is still connected.
 *
 * @param my_client Pointer to the `Client`.
 * @param i Index in `all_fd` corresponding to the client.
 * 
 * @return Returns -1 if the client is disconnected and CGI cleaned up, 0 otherwise.
 */
int Monitor::searchClient(Client *my_client, int i)
{
		bool client_still_connected = false;
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (&it->second == my_client)
		{
			client_still_connected = true;
			break;
		}
	}
	
	if (!client_still_connected)
	{
		std::cerr << "[POLLIN_CGI] Client disconnected, cleaning up CGI without sending response" << std::endl;
		
		pid_t cgi_pid = my_client->getCgiPid();
		if (cgi_pid > 0)
		{
		   	kill(cgi_pid, SIGKILL);
			waitpid(cgi_pid, NULL, WNOHANG);
		}
		std::map<int, Client*>::iterator it_temp = tab_CGI.find(all_fd[i].fd);
		if (it_temp != tab_CGI.end())
		{
			if (tab_CGI.find(it_temp->second->getPipeOut()) != tab_CGI.end())
				tab_CGI.erase(it_temp->second->getPipeOut());
			tab_CGI.erase(all_fd[i].fd);
		}
		return (-1);
	}
	return (0);
}

/**
 * @brief
 * Default constructor for `Monitor`.
 */
Monitor::Monitor() {}

/**
 * @brief
 * Destructor for `Monitor`.
 */
Monitor::~Monitor() {}

/**
 * @brief
 * Copy constructor for `Monitor`.
 *
 * @param copy The `Monitor` to copy.
 */
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

/**
 * @brief
 * Assignment operator for `Monitor`.
 *
 * @param copy Source `Monitor`.
 * 
 * @return Returns a reference to the assigned `Monitor`.
 */
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

/**
 * @brief
 * Adds a `fd` to the `all_fd` array with specific `events`.
 *
 * @param fd File descriptor to add.
 * @param events Associated events (`POLLIN` or `POLLOUT`).
 */
void   Monitor::add_fd(int &fd, int events)
{
	all_fd[this->nb_fd].fd = fd;
	all_fd[this->nb_fd].events = POLLIN | POLLHUP;
	if (events > 1)
		all_fd[this->nb_fd].events = POLLOUT;
	this->nb_fd++;
}

/**
 * @brief
 * Constructor that initializes the `Monitor` with a table of `Socket`.
 *
 * @param tab Vector of `Socket*` to monitor.
 */
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
		all_sockets.insert(std::make_pair((*it)->getFd(), *it));
		nb_fd++;
		it++;
	}
	nb_fd_server = nb_fd;
}

/**
 * @brief
 * Adds a new `Client` to the `clients` map and sets its IP and port.
 *
 * @param fd Client file descriptor.
 * @param ip Client IP address.
 * @param port Client port.
 * @param fd_server Server file descriptor associated with this client.
 */
void Monitor::add_client(int fd, in_addr_t ip, in_port_t port, int fd_server)
{
	Client  nouveau(all_sockets.at(fd_server));
	uint32_t	ip_adress = ntohl(ip);
	uint16_t	port_adress = ntohs(port);
	std::ostringstream	oss;
	std::string ip_str;
	std::string port_str;
	unsigned char bytes[4];

	oss << port_adress;
	port_str = oss.str();
	bytes[0] = ip_adress >> 24 & 0xFF; 
	bytes[1] = ip_adress >> 16 & 0xFF;
	bytes[2] = ip_adress >> 8 & 0xFF;
	bytes[3] = ip_adress & 0xFF;

	oss.str("");
	oss.clear();
	for (int i = 0; i < 4; i++)
	{
		oss << (int)bytes[i];
		if (i < 3)
			oss << ".";
	}
	ip_str = oss.str();
	nouveau.setbasic(ip_str, port_str);
	clients.insert(std::pair<int, Client>(fd, nouveau));
}


/**
 * @brief
 * Removes a `Client` from the `clients` map and cleans up associated CGI pipes and processes.
 *
 * @param i Index in `all_fd` corresponding to the client.
 *
 * @return Always returns 0.
 */
int Monitor::deconnexion(int i)
{
	const int client_fd = all_fd[i].fd;
	std::map<int, Client>::iterator it_client = clients.find(client_fd);
	Client *client_ptr = (it_client != clients.end()) ? &it_client->second : 0;

	if (client_ptr)
		client_ptr->disconnected();

	int pipe_in  = client_ptr ? client_ptr->getPipeIn()  : -1;
	int pipe_out = client_ptr ? client_ptr->getPipeOut() : -1;
	pid_t cgi_pid = client_ptr ? client_ptr->getCgiPid() : -1;
	
	if (pipe_in > 0 && tab_CGI.find(pipe_in) != tab_CGI.end())
		tab_CGI.erase(pipe_in);
	if (pipe_out > 0 && tab_CGI.find(pipe_out) != tab_CGI.end())
		tab_CGI.erase(pipe_out);
	
	if (cgi_pid > 0)
	{
		kill(cgi_pid, SIGKILL);
		usleep (500);
		waitpid(cgi_pid, NULL, WNOHANG);
	}
	std::vector<size_t> indices_to_remove;
	for (size_t j = 0; j < nb_fd; j++)
	{
		int fdj = all_fd[j].fd;
		if (fdj == pipe_in || fdj == pipe_out)
			indices_to_remove.push_back(j);
	}    
	for (int k = indices_to_remove.size() - 1; k >= 0; k--)
		remove_fd(indices_to_remove[k]);
	clients.erase(client_fd);
	close(client_fd);
	all_fd[i] = all_fd[nb_fd - 1];
	all_fd[nb_fd - 1].fd = -1;
	nb_fd--;
	return 0;
}

/**
 * @brief
 * Tests the result of `recv` to handle errors and connection termination.
 *
 * @param count Number of bytes read.
 * 
 * @return 1 if read OK, 2 if non-blocking with no data, 0 otherwise.
 */
int	 Monitor::test_read(ssize_t count)
{
	if (count > 0)
		return (1);
	else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return (2);
	return (0);
}

/**
 * @brief
 * Reads data from a `Client` and stores it in its object.
 *
 * @param i Index of the client in `all_fd`.
 * 
 * @return 1 if success, otherwise disconnects the client.
 */
int	Monitor::new_request(int i)
{
	ssize_t count;
	char buf[1024];
	std::string buf_final;
	int	result;
	
	while (42)
	{
		count = recv(all_fd[i].fd, buf, sizeof(buf), 0);
		result = test_read(count);
		if (!result)
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

/**
 * @brief
 * Removes a `fd` from the `all_fd` array.
 *
 * @param index Index of the `fd` in `all_fd`.
 */
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

/**
 * @brief
 * Accepts a new client on a server `fd` and adds it to `clients`.
 *
 * @param i Index of the server `fd` in `all_fd`.
 * 
 * @return 0 if success, 1 otherwise.
 */
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
	add_client(all_fd[nb_fd].fd, address.sin_addr.s_addr, address.sin_port, all_fd[i].fd);
	int ancien_flags = fcntl (all_fd[nb_fd].fd, F_GETFL);
	fcntl(all_fd[nb_fd].fd, F_SETFL, ancien_flags | O_NONBLOCK);
	nb_fd++;
	return (0);
}

/**
 * @brief
 * Checks timeouts for ongoing CGI processes and handles clients and responses accordingly.
 */
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
		if (resultat > 3.0)
		{
			kill(it_tab_cgi->second->getCgiPid(), SIGKILL);
			std::cout << "TIMEOUT" << std::endl;
			usleep(500);
			Response new_response;
			makeResponse(new_response, 504, readFile("config/www/errors/504.html"), "");
			it_tab_cgi->second->setResponse(it_tab_cgi->second->handler.responseToString(new_response));
			it_tab_cgi->second->setResponseGenerate(true);
			it_tab_cgi->second->setOutCGI();
			it_tab_cgi->second->setaddPipeToPoll(false);
			waitpid(it_tab_cgi->second->getCgiPid(), NULL, WNOHANG);
			usleep(50);
			reactive_pollout(it_tab_cgi->second, it_tab_cgi->second->getPipeIn(), it_tab_cgi->second->getPipeOut(), true);
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

/**
 * @brief
 * Reactivates `POLLOUT` for a client or removes CGI pipes after timeout.
 *
 * @param my_client Pointer to the client.
 * @param PipeIn CGI read pipe `fd`.
 * @param PipeOut CGI write pipe `fd`.
 * @param timeout True if called during a timeout.
 */
void	Monitor::reactive_pollout(Client *my_client, int PipeIn, int PipeOut, bool timeout)
{
	int	fd_current = -1;
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (&(it->second) == my_client)
		{
			fd_current = it->first;
			break;
		}
	}
	int	count = 0;
	bool pass = false;
	for (size_t j = 0; j < nb_fd;)
	{
		if (all_fd[j].fd == fd_current)
		{
			all_fd[j].events |= POLLOUT;
			pass = true;
		}
		if (timeout && (all_fd[j].fd == PipeOut || all_fd[j].fd == PipeIn))
		{
			remove_fd(j);
			count++;
		}
		else
			j++;
		if (pass && count > 1)
			break;
	}
}

/**
 * @brief
 * Removes CGI `fd`s from the `all_fd` array.
 *
 * @param my_client Pointer to the client.
 * @param y Determines which pipes to remove: 1 = out, 2 = in, 3 = both.
 */
void	Monitor::remove_fd_CGI(Client *my_client, int y)
{
	int PipeIn = my_client->getPipeIn();
	int PipeOut = my_client->getPipeOut();
 	for (size_t i = nb_fd_server; i < nb_fd;)
	{
		bool removed = false;
		
		if (all_fd[i].fd == PipeOut && (y == 1 || y == 3))
		{
			remove_fd(i);
			my_client->setPipeOut(-1);
			removed = true;
			continue ;
		}
		if (all_fd[i].fd == PipeIn && (y == 2 || y == 3))
		{
			remove_fd(i);
			my_client->setPipeIn(-1);
			removed = true;
			continue ;
		}
		if (!removed)
			i++;
	}
}

/**
 * @brief
 * Handles writing the client body to a CGI pipe.
 *
 * @param i Index in `all_fd`.
 * @param my_client Pointer to the client.
 *
 * @return -1 if pipe is done or error, 0 otherwise.
 */
int	Monitor::pollout_CGI(int i, Client *my_client)
{
	if ((all_fd[i].revents & POLLOUT || all_fd[i].revents & POLLHUP) && all_fd[i].fd == my_client->getPipeOut() && my_client->getPipeOut() > 0)
	{
		const std::string& body = my_client->getBody();
		int	reste = body.size() - my_client->getOffsetBodyCgi(); 
		if (reste > 0)
		{
			int status;
			pid_t result = waitpid(my_client->getCgiPid(), &status, WNOHANG);
			if (result != 0)
			{
				std::cerr << "CGI process died before receiving body" << std::endl;
				return -1;
			}
			std::cout << "ca ecrit" << std::endl;
			int	nb_written = write(all_fd[i].fd, body.c_str() + my_client->getOffsetBodyCgi(), reste);
			if (nb_written > 0)
				return (my_client->addCgiBodyOffset(nb_written), -1);
			if (nb_written == 0)
				return (-1);
			if (nb_written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
				return (-1);
			else
			{
				kill(my_client->getCgiPid(), SIGKILL);
				waitpid(my_client->getCgiPid(), NULL, WNOHANG);
				std::cerr << "Erreur ecriture pipe CGI: " << strerror(errno) << std::endl;
				tab_CGI.erase(all_fd[i].fd);
				remove_fd_CGI(my_client, 3);
				my_client->setOutCGI();
				i++;
				return (-1);
			}
		}
		else
		{
			tab_CGI.erase(all_fd[i].fd);
			remove_fd_CGI(my_client, 1);
			return (-1);
		}
	}
	return (0);
}

/**
 * @brief
 * Handles reading CGI output from a pipe.
 *
 * @param i Index in `all_fd`.
 * @param my_client Pointer to the client.
 *
 * @return -1 if pipe is done or error, 0 otherwise.
 */
int	Monitor::pollin_CGI(int &i, Client *my_client)
{

	if ((all_fd[i].revents & POLLIN || all_fd[i].revents & POLLHUP) && all_fd[i].fd == my_client->getPipeIn())
	{
		char buffer[4096];
		ssize_t nb_read = -1;
		if (all_fd[i].fd > 0)
			nb_read = read(all_fd[i].fd, &buffer, sizeof(buffer));
		if (nb_read > 0)
		{
			my_client->addCgiOutput(std::string(buffer, nb_read));
			return (-1);
		}
		if (nb_read == 0 || (nb_read < 0 && all_fd[i].revents & POLLHUP))
		{

			Response new_response = parseCGIOutput(my_client->getCgiOutput());
			my_client->setResponse(my_client->handler.responseToString(new_response));
			my_client->setResponseGenerate(true);
			usleep(1000);
			waitpid(my_client->getCgiPid(), NULL, WNOHANG);
			reactive_pollout(my_client, my_client->getPipeIn(), my_client->getPipeOut(), false);
			std::map<int, Client*>::iterator it_temp = tab_CGI.find(all_fd[i].fd);  
			if (it_temp != tab_CGI.end())
			{
				if (tab_CGI.find(it_temp->second->getPipeOut()) != tab_CGI.end())
					tab_CGI.erase(it_temp->second->getPipeOut());
				tab_CGI.erase(all_fd[i].fd);
			}
			remove_fd_CGI(my_client, 3);
			my_client->resetAfterCGI();
			my_client->setOutCGI();
			my_client->setaddPipeToPoll(false);
			return (-1);
		}
		if (nb_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
			return (-1);
		else if (nb_read < 0)
		{
			kill(my_client->getCgiPid(), SIGKILL);
			usleep(3000);
			waitpid(my_client->getCgiPid(), NULL, WNOHANG);
			Response new_response;
			makeResponse(new_response, 504, readFile("config/www/errors/504.html"), "");
			my_client->setResponse(my_client->handler.responseToString(new_response));
			my_client->setResponseGenerate(true);
			my_client->setOutCGI();
			my_client->setaddPipeToPoll(false);
			my_client->resetAfterCGI();
			reactive_pollout(my_client, my_client->getPipeIn(), my_client->getPipeOut(), false);
			std::map<int, Client*>::iterator it_temp = tab_CGI.find(all_fd[i].fd);  
			if (it_temp != tab_CGI.end())
			{
				if (tab_CGI.find(it_temp->second->getPipeOut()) != tab_CGI.end())
					tab_CGI.erase(it_temp->second->getPipeOut());
				tab_CGI.erase(all_fd[i].fd);
			}
			remove_fd_CGI(my_client, 3);
			return (-1);
		}
	}
	return (0);
}

/**
 * @brief
 * Main mechanism to handle CGI pipes (input/output).
 *
 * @param i Index in `all_fd`.
 *
 * @return -1 if client disconnected or error, 0 otherwise.
 */
int	Monitor::CGI_engine(int i)
{
	std::map<int, Client *>::iterator it = tab_CGI.find(all_fd[i].fd);
	if (it == tab_CGI.end())
		return 0;
	std::cerr << "CGI_engine: fd " << all_fd[i].fd << " found in tab_CGI" << std::endl;
	Client *my_client = it->second;
	bool client_exists = false;
	for (std::map<int, Client>::iterator it_c = clients.begin(); it_c != clients.end(); ++it_c)
	{
		if (&it_c->second == my_client)
		{
			client_exists = true;
			break;
		}
	}
	std::cerr << "CGI_engine: client_exists = " << client_exists << std::endl;
	if (searchClient(my_client, i) < 0)
				return (-1);
	if (it != tab_CGI.end())
	{
		if (pollout_CGI(i, my_client) < 0)
			return (-1);
		if (pollin_CGI(i, my_client) < 0)
			return(-1);
	}
	return (0);
}

/**
 * @brief
 * Adds a client's CGI pipes to the `all_fd` array for polling.
 *
 * @param current Pointer to the client.
 * @param i Index in `all_fd`.
 */
void	Monitor::AddCgiPollFd(Client *current, int i)
{
	if (current->getInCGI() == true && !current->getaddPipeToPoll())
	{
		int	PipeIn = current->getPipeIn();
		int	PipeOut = current->getPipeOut();
		if (PipeOut > 0)
		{
			int	flags = fcntl(PipeOut, F_GETFL);
			fcntl(PipeOut, F_SETFL, flags | O_NONBLOCK);
			add_fd(PipeOut, 2);
			tab_CGI.insert(std::make_pair(PipeOut, current));
		}
		if (PipeIn > 0)
		{
			int	flags = fcntl(PipeIn, F_GETFL);
			fcntl(PipeIn, F_SETFL, flags | O_NONBLOCK);
			add_fd(PipeIn, 1);
			tab_CGI.insert(std::make_pair(PipeIn, current));
		}
		if (current->getPipeIn() > 0)
		{
			current->setaddPipeToPoll(true);
			all_fd[i].events = POLLIN;
		}
	}
}

/**
 * @brief
 * Handles the aftermath of a `send` call on a client.
 *
 * @param current Pointer to the client.
 * @param i Index of the client in `all_fd`.
 * @param nb_send Number of bytes sent.
 */
void	Monitor::AfterSend(Client *current, int i, int nb_send)
{
	std::cout << "AfterSend called with nb_send: " << nb_send << std::endl;
	if (nb_send < 0)
	{
		if (errno == EPIPE || errno == ECONNRESET)
		{
			deconnexion(i);
			return;
		}
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			perror("send");
			deconnexion(i);
			return;
		}
	}

	if (nb_send > 0)
		current->AddOffset(nb_send);
	if (current->getOffset() >= current->getReponse().length() && !current->getInCGI() 
		&& current->getReponse().length() > 0)
	{
		all_fd[i].events = POLLIN;
		current->setResponse("");
		current->setResponseGenerate(false);
		current->resetAfterCGI();
		current->setOutCGI();
		current->resetRequestState();
		current->ResetCgiOutput();
	}
}
/**
 * @brief
 * Main server loop that polls sockets, accepts connections, and manages CGI processes.
 */
void	Monitor::Monitoring()
{
	int poll_return;
	std::map<std::string, Session> g_sessions;
	std::string uploadsPath = "./config/www/uploads";

	std::cout << "Lancement du server" << std::endl;
	findHtmlFiles("close", "./config");
	if (!pathExists(uploadsPath) && mkdir(uploadsPath.c_str(), 0755) == -1)
		std::cerr << "Failed to recreate " << uploadsPath << std::endl;
	while (on)
	{
		poll_return = poll(this->all_fd, nb_fd, 15);
		Timeout();
		if (poll_return == 0)
			continue ;
		else if (poll_return < 0)
		{
			if (errno == EINTR)
				break;
			throw MonitorError();
		}
		else if (poll_return > 0)
		{
			for (size_t i = 0; i < nb_fd;)
			{
				std::map<int, Client>::iterator it_client = clients.find(all_fd[i].fd);
				bool client_disconnected = false;
				if (all_fd[i].fd < 0)
				{
					i++;
					continue;
				}
				if (CGI_engine(i) < 0 || all_fd[i].fd < 0 || all_fd[i].revents & POLLNVAL)
				{
					i++;
					continue;
				}
				if (i >= nb_fd_server && it_client == clients.end() && tab_CGI.find(all_fd[i].fd) == tab_CGI.end())
				{
					std::cerr << "Cleaning orphan fd: " << all_fd[i].fd << std::endl;
					remove_fd(i);
					continue;
				}
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
				if (all_fd[i].revents & POLLHUP)
				{
					if (i >= nb_fd_server)
					{
						if (all_fd[i].revents & POLLIN)
							client_disconnected = new_request(i);
						if (client_disconnected)
							deconnexion(i);
					}
					else
						i++;
					continue;
				}				
				if (all_fd[i].revents & POLLIN)
				{
					if (i < nb_fd_server)
					{
						new_clients(i++);
						continue;
					}
					else if (i >= nb_fd_server && !new_request(i))
						continue;
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
						Response	structResponse = it_client->second.handler.handleRequest(request, g_sessions, &it_client->second);
						updateClientCookies(it_client->second, structResponse);
						if (it_client->second.getInCGI())
							it_client->second.setResponseGenerate(true);
						else
						{
							it_client->second.setResponse(it_client->second.handler.responseToString(structResponse));
							it_client->second.setResponseGenerate(true);
						}
					}
					AddCgiPollFd(&it_client->second, i);
					it_client = clients.find(all_fd[i].fd);
					if (it_client == clients.end())
					{
						deconnexion(i);
						continue;
					}
					if (it_client->second.getInCGI() == false && all_fd[i].fd > 0 &&  it_client->second.getReponse() != "")
					{
						int error = 0;
						socklen_t len = sizeof(error);
						if (getsockopt(all_fd[i].fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0)
						{
							std::cerr << "Socket error detected before send: " << strerror(error) << std::endl;
							deconnexion(i);
							continue;
						}
						std::cout << "SEND" << std::endl;
						nb_send = send(all_fd[i].fd,  it_client->second.getReponse().c_str() + offset, it_client->second.getReponse().length() - offset, 0);
						AfterSend(&it_client->second, i, nb_send);
						it_client = clients.find(all_fd[i].fd);
						if (it_client == clients.end())
						{
							deconnexion(i);
							continue;
						}
					}
				}
				i++;
			}
		}
	}
	findHtmlFiles("open", "./config");
	resetUploadsDir(uploadsPath);
	clean_CGI();
}

/**
 * @brief
 * Updates a client's cookies based on a response.
 *
 * @param client Client to update.
 * @param resp Response to analyze for cookies.
 */
static void parseSetCookie(const std::string &header, std::string &name, std::string &value)
{
	size_t start = header.find("Set-Cookie: ");
	if (start == std::string::npos)
		return;
	start += strlen("Set-Cookie: ");
	size_t end = header.find(';', start);
	std::string cookie_pair = header.substr(start, end - start);

	size_t eq = cookie_pair.find('=');
	if (eq != std::string::npos)
	{
		name = cookie_pair.substr(0, eq);
		value = cookie_pair.substr(eq + 1);
	}
}

/**
 * @brief
 * Cleans all remaining CGI processes and closes their pipes.
 */
void Monitor::updateClientCookies(Client &client, const Response &resp)
{
	if (!client.getCookies().empty())
		return;
	for (std::vector<std::string>::const_iterator it = resp.headers.begin(); it != resp.headers.end(); ++it)
	{
		std::string name, value;
		parseSetCookie(*it, name, value);
		if (!name.empty() && !value.empty())
			client.setCookies(name, value);
	}
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
			kill(pid, SIGKILL);
			usleep(100000);
			kill(pid, SIGKILL);
			waitpid(pid, NULL, WNOHANG);
		}
	}
}
