#ifndef MONITOR_HPP

# define MONITOR_HPP

# include "Client.hpp"
# include "Server.hpp"

/**
 * @brief
 * Manages all server and client file descriptors using a single `poll()` loop.
 * 
 * This class is responsible for:
 * - Monitoring server sockets and accepting new clients
 * - Handling client requests and responses
 * - Managing CGI pipes and timeouts
 * - Ensuring only one poll instance is used (project constraint)
 * 
 * @param nb_fd			Total number of monitored file descriptors.
 * @param nb_fd_server	Number of server sockets.
 * @param clients		Map of connected clients indexed by their file descriptor.
 * @param all_sockets	Map of server sockets indexed by their file descriptor.
 * @param all_fd		Array of pollfd structures.
 * @param tab_CGI		Map of CGI-related clients indexed by file descriptor.
 */
class Monitor
{
private :
	size_t					nb_fd;
	size_t					nb_fd_server;
	std::map<int, Client>	clients;
	std::map<int, Socket *>	all_sockets;
	struct  pollfd			all_fd[200000];
	std::map<int, Client *>	tab_CGI;

	// Internal helpers

	Monitor();
	void 	add_client(int fd, in_addr_t ip, in_port_t port, int fd_server);
	int		new_clients(int i);
	int		new_request(int i);
	int		test_read(ssize_t count);
	int		deconnexion(int i);
	void 	remove_fd(int index);
	void	Timeout();
	void	clean_CGI();

	// CGI handling

	int		CGI_engine(int current_fd);
	int		pollout_CGI(int i, Client *my_client);
	int		pollin_CGI(int &i, Client *my_client);
	void	remove_fd_CGI(Client *my_client, int y);
	void	reactive_pollout(Client *my_client, int PipeIn, int PipeOut, bool timeout);
	void	AddCgiPollFd(Client *current, int i);

	// Client lifecycle

	void	AfterSend(Client *current, int i, int nb_send);
	void 	updateClientCookies(Client &client, const Response &resp);
	int 	searchClient(Client *my_client, int i);
public :
	Monitor(std::vector<Socket *> tab);
	~Monitor();
	Monitor(const Monitor &copy);
	Monitor &operator=(const Monitor &copy);

	// Public interface

	void	add_fd(int &fd, int event);
	void	Monitoring();

	class MonitorError : public std::exception
	{
		const char *what() const throw();
	};
};

#endif
