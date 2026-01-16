/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Monitor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 20:34:42 by abelmoha          #+#    #+#             */
/*   Updated: 2026/01/16 21:58:57 by abelmoha         ###   ########.fr       */
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
		std::cerr << "[pollInCgi] Client disconnected, cleaning up CGI without sending response" << std::endl;
		
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
void   Monitor::addFd(int &fd, int events)
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
	for (size_t i = nb_fd; i < NUMBERS_CLIENTS; i++)
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
	Socket *serverSock = NULL;
	std::map<int, Socket *>::iterator itSock = all_sockets.find(fd_server);
	if (itSock != all_sockets.end())
		serverSock = itSock->second;
	else if (!all_sockets.empty())
	{
		std::cerr << "Warning: server fd " << fd_server << " not found in all_sockets. Using first server socket as fallback." << std::endl;
		serverSock = all_sockets.begin()->second;
	}
	else
	{
		std::cerr << "Error: no server sockets registered; cannot attach client." << std::endl;
		close(fd);
		return;
	}
	Client  nouveau(serverSock);
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
	nouveau.setBasic(ip_str, port_str);
	// Remove any stale client with the same fd (shouldn't happen, but safety)
	if (clients.find(fd) != clients.end())
	{
		std::cerr << "WARNING: Client fd " << fd << " already exists in map! Removing stale entry." << std::endl;
		clients.erase(fd);
	}
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
int Monitor::disconnect(int i)
{
	if (i < (int)nb_fd_server)
	{
		std::cerr << "ERROR: Attempted to disconnect server socket at index " << i << std::endl;
		return 0;
	}
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
	for (size_t j = nb_fd_server; j < nb_fd; j++)
	{
		int fdj = all_fd[j].fd;
		if (fdj == pipe_in || fdj == pipe_out)
			indices_to_remove.push_back(j);
	}    
	for (int k = indices_to_remove.size() - 1; k >= 0; k--)
		removeFd(indices_to_remove[k]);
	clients.erase(client_fd);
	
	if (all_sockets.find(client_fd) != all_sockets.end())
	{
		std::cerr << "ERROR: fd " << client_fd << " is a server socket, not closing!" << std::endl;
		return 0;
	}
	close(client_fd);
	while (nb_fd > nb_fd_server && all_fd[nb_fd - 1].fd <= 0)
		nb_fd--;
	if ((size_t)i < nb_fd - 1 && nb_fd > nb_fd_server)
	{
		all_fd[i] = all_fd[nb_fd - 1];
		all_fd[nb_fd - 1].fd = -1;
		nb_fd--;
	}
	else if ((size_t)i == nb_fd - 1)
	{
		all_fd[i].fd = -1;
		nb_fd--;
	}
	else
	{
		all_fd[i].fd = -1;
	}
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
int	 Monitor::testRead(ssize_t count)
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
int	Monitor::newRequest(int i)
{
	ssize_t count;
	char buf[1024];
	std::string buf_final;
	int	result;
	
	while (42)
	{
		count = recv(all_fd[i].fd, buf, sizeof(buf), 0);
		result = testRead(count);
		if (!result)
			return (disconnect(i));
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
 * @parm index Index of the `fd` in `all_fd`.
 */
void Monitor::removeFd(int index)
{
	if (index < (int)nb_fd_server)
	{
		std::cerr << "ERROR: Attempted to remove server socket at index " << index << std::endl;
		return;
	}
	if (all_fd[index].fd > 0)
	{
		if (all_sockets.find(all_fd[index].fd) != all_sockets.end())
		{
			std::cerr << "ERROR: fd " << all_fd[index].fd << " is a server socket, not removing!" << std::endl;
			return;
		}
		close(all_fd[index].fd);
	}
	while (nb_fd > nb_fd_server && all_fd[nb_fd - 1].fd <= 0)
		nb_fd--;
	if ((size_t)index < nb_fd - 1 && nb_fd > nb_fd_server)
	{
		all_fd[index] = all_fd[nb_fd - 1];
		all_fd[nb_fd - 1].fd = -1;
		nb_fd--;
	}
	else if ((size_t)index == nb_fd - 1)
	{
		all_fd[index].fd = -1;
		nb_fd--;
	}
	else
	{
		all_fd[index].fd = -1;
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
int	Monitor::newClients(int i)
{
	if (i >= (int)nb_fd_server)
	{
		std::cerr << "ERROR: newClients called on non-server index " << i << std::endl;
		return 1;
	}
	
	size_t	accepted = 0;
	while (true)
	{
		struct sockaddr_in address;
		socklen_t addrlen = sizeof(address);
		int client_fd = accept(all_fd[i].fd, (sockaddr *)&address, &addrlen);
		if (client_fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EINVAL)
			{
				std::cerr << "FATAL: Server socket fd " << all_fd[i].fd << " is no longer listening! Disabling POLLIN." << std::endl;
				all_fd[i].events = 0;
				return 1;
			}
			std::cerr << "accept() error: " << std::strerror(errno) << " (errno=" << errno << ")" << std::endl;
			if (errno == EMFILE || errno == ENFILE)
				std::cerr << "Hint: process/system file descriptor limit reached." << std::endl;
			else if (errno == ENOBUFS || errno == ENOMEM)
				std::cerr << "Kernel buffer/memory exhausted under load" << std::endl;
			break;
		}
		all_fd[nb_fd].fd = client_fd;
		all_fd[nb_fd].events = POLLIN;	
		add_client(client_fd, address.sin_addr.s_addr, address.sin_port, all_fd[i].fd);
		int ancien_flags = fcntl(client_fd, F_GETFL);
		fcntl(client_fd, F_SETFL, ancien_flags | O_NONBLOCK);
		nb_fd++;
		accepted++;
	}
	return ((accepted > 0) ? 0 : 1);
}

/**
 * @brief
 * Checks timeouts for ongoing CGI processes and handles clients and responses accordingly.
 */
void	Monitor::timeout()
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
			std::cout << "Timeout" << std::endl;
			usleep(500);
			Response new_response;
			makeResponse(new_response, 504, readFile("config/www/errors/504.html"), "");
			it_tab_cgi->second->setResponse(it_tab_cgi->second->handler.responseToString(new_response));
			it_tab_cgi->second->setResponseGenerate(true);
			it_tab_cgi->second->setOutCgi();
			it_tab_cgi->second->setAddPipeToPoll(false);
			waitpid(it_tab_cgi->second->getCgiPid(), NULL, WNOHANG);
			usleep(50);
			reactivePollOut(it_tab_cgi->second, it_tab_cgi->second->getPipeIn(), it_tab_cgi->second->getPipeOut(), true);
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
void	Monitor::reactivePollOut(Client *my_client, int PipeIn, int PipeOut, bool timeout)
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
			removeFd(j);
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
void	Monitor::removeFd_CGI(Client *my_client, int y)
{
	int PipeIn = my_client->getPipeIn();
	int PipeOut = my_client->getPipeOut();
 	for (size_t i = nb_fd_server; i < nb_fd;)
	{
		bool removed = false;
		
		if (all_fd[i].fd == PipeOut && (y == 1 || y == 3))
		{
			removeFd(i);
			my_client->setPipeOut(-1);
			removed = true;
			continue ;
		}
		if (all_fd[i].fd == PipeIn && (y == 2 || y == 3))
		{
			removeFd(i);
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
int	Monitor::pollOutCgi(int i, Client *my_client)
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
				std::cerr << "Error while writting in CGI pipe : " << strerror(errno) << std::endl;
				tab_CGI.erase(all_fd[i].fd);
				removeFd_CGI(my_client, 3);
				my_client->setOutCgi();
				i++;
				return (-1);
			}
		}
		else
		{
			tab_CGI.erase(all_fd[i].fd);
			removeFd_CGI(my_client, 1);
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
int	Monitor::pollInCgi(int &i, Client *my_client)
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

			Response new_response = parseCgiOutput(my_client->getCgiOutput());
			my_client->setResponse(my_client->handler.responseToString(new_response));
			my_client->setResponseGenerate(true);
			usleep(1000);
			waitpid(my_client->getCgiPid(), NULL, WNOHANG);
			reactivePollOut(my_client, my_client->getPipeIn(), my_client->getPipeOut(), false);
			std::map<int, Client*>::iterator it_temp = tab_CGI.find(all_fd[i].fd);  
			if (it_temp != tab_CGI.end())
			{
				if (tab_CGI.find(it_temp->second->getPipeOut()) != tab_CGI.end())
					tab_CGI.erase(it_temp->second->getPipeOut());
				tab_CGI.erase(all_fd[i].fd);
			}
			removeFd_CGI(my_client, 3);
			my_client->resetAfterCGI();
			my_client->setOutCgi();
			my_client->setAddPipeToPoll(false);
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
			my_client->setOutCgi();
			my_client->setAddPipeToPoll(false);
			my_client->resetAfterCGI();
			reactivePollOut(my_client, my_client->getPipeIn(), my_client->getPipeOut(), false);
			std::map<int, Client*>::iterator it_temp = tab_CGI.find(all_fd[i].fd);  
			if (it_temp != tab_CGI.end())
			{
				if (tab_CGI.find(it_temp->second->getPipeOut()) != tab_CGI.end())
					tab_CGI.erase(it_temp->second->getPipeOut());
				tab_CGI.erase(all_fd[i].fd);
			}
			removeFd_CGI(my_client, 3);
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
	Client *my_client = it->second;
	for (std::map<int, Client>::iterator it_c = clients.begin(); it_c != clients.end(); ++it_c)
		if (&it_c->second == my_client)
			break;
	if (searchClient(my_client, i) < 0)
				return (-1);
	if (it != tab_CGI.end())
	{
		if (pollOutCgi(i, my_client) < 0)
			return (-1);
		if (pollInCgi(i, my_client) < 0)
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
void	Monitor::addCgiPollFd(Client *current, int i)
{
	if (current->getInCGI() == true && !current->getaddPipeToPoll())
	{
		int	PipeIn = current->getPipeIn();
		int	PipeOut = current->getPipeOut();
		if (PipeOut > 0)
		{
			int	flags = fcntl(PipeOut, F_GETFL);
			fcntl(PipeOut, F_SETFL, flags | O_NONBLOCK);
			addFd(PipeOut, 2);
			tab_CGI.insert(std::make_pair(PipeOut, current));
		}
		if (PipeIn > 0)
		{
			int	flags = fcntl(PipeIn, F_GETFL);
			fcntl(PipeIn, F_SETFL, flags | O_NONBLOCK);
			addFd(PipeIn, 1);
			tab_CGI.insert(std::make_pair(PipeIn, current));
		}
		if (current->getPipeIn() > 0)
		{
			current->setAddPipeToPoll(true);
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
void	Monitor::afterSend(Client *current, int i, int nb_send)
{
	if (nb_send < 0)
	{
		if (errno == EPIPE || errno == ECONNRESET)
		{
			disconnect(i);
			return;
		}
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			perror("send");
			disconnect(i);
			return;
		}
	}
	if (nb_send > 0)
		current->addOffset(nb_send);
	if (current->getOffset() >= current->getReponse().length() && !current->getInCGI() 
		&& current->getReponse().length() > 0)
	{
		disconnect(i);
		return;
	}
}
/**
 * @brief
 * Main server loop that polls sockets, accepts connections, and manages CGI processes.
 */
void	Monitor::monitoring()
{
	int poll_return;
	std::map<std::string, Session> g_sessions;
	std::string uploadsPath = "./config/www/uploads";

	std::cout << "Server started" << std::endl;
	findHtmlFiles("close", "./config");
	if (!pathDirectoryExists(uploadsPath) && mkdir(uploadsPath.c_str(), 0755) == -1)
		std::cerr << "Failed to recreate " << uploadsPath << std::endl;
	while (on)
	{
		poll_return = poll(this->all_fd, nb_fd, 15);
		timeout();
		if (poll_return == 0)
			continue ;
		else if (poll_return < 0)
		{
			if (errno == EINTR)
				break;
			if (errno == EINVAL)
				continue;  
			std::cerr << "poll() error: " << strerror(errno) << std::endl;
			continue;
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
					removeFd(i);
					continue;
				}
				if (all_fd[i].revents & POLLERR)
				{
					if (i >= nb_fd_server)
					{
						disconnect(i);
						continue;
					}
					else
					{
						std::cout << "Issue with server's socket" << std::endl;
						i++;
					}
				}
				if (all_fd[i].revents & POLLHUP)
				{
					if (i >= nb_fd_server)
					{
						if (all_fd[i].revents & POLLIN)
							client_disconnected = newRequest(i);
						if (client_disconnected)
							disconnect(i);
					}
					else
						i++;
					continue;
				}				
				if (all_fd[i].revents & POLLIN)
				{
					if (i < nb_fd_server)
					{
						newClients(i);
						i++;
						continue;
					}
					else if (i >= nb_fd_server && !newRequest(i))
						continue;
				}
				if (i >= this->nb_fd_server && all_fd[i].fd > 0 && it_client != clients.end() && it_client->second.getFinishRequest())
					all_fd[i].events |= POLLOUT;
				if (all_fd[i].revents & POLLOUT && i >= nb_fd_server && it_client != clients.end())
				{
					int	  nb_send = 0;
					size_t		offset;

					offset = it_client->second.getOffset();
					if (it_client->second.getFinishRequest() && offset == 0 && it_client->second.getInCGI() == false && !it_client->second.getResponseGenerate())
					{
						Request request = it_client->second.ExtractRequest();
						// Safety check: skip if request is incomplete
						if (request.method.empty() || request.path.empty())
						{
							i++;
							continue;
						}
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
					addCgiPollFd(&it_client->second, i);
					it_client = clients.find(all_fd[i].fd);
					if (it_client == clients.end())
					{
						disconnect(i);
						continue;
					}
					if (it_client->second.getInCGI() == false && all_fd[i].fd > 0 &&  it_client->second.getReponse() != "")
					{
						int error = 0;
						socklen_t len = sizeof(error);
						if (getsockopt(all_fd[i].fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0)
						{
							std::cerr << "Socket error detected before send: " << strerror(error) << std::endl;
							disconnect(i);
							continue;
						}
						std::cout << "SEND" << std::endl;
						nb_send = send(all_fd[i].fd,  it_client->second.getReponse().c_str() + offset, it_client->second.getReponse().length() - offset, 0);
						afterSend(&it_client->second, i, nb_send);
						it_client = clients.find(all_fd[i].fd);
						if (it_client == clients.end())
						{
							disconnect(i);
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
	cleanCgi();
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

void	Monitor::cleanCgi()
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
