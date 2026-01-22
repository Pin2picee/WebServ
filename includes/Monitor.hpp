#ifndef MONITOR_HPP

# define MONITOR_HPP

# include "Client.hpp"
# include "Server.hpp"

/**
 * @brief
 * Manages all server and client file descriptors using a single `poll()` loop.
 * 
 * This class is responsible for:
 * - monitoring server sockets and accepting new clients
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
	std::map<int, Socket *>	all_sockets;//ma map avec cle = fd, valeur = obj Socket;
	struct pollfd			all_fd[NUMBERS_CLIENTS];// dois tout avoir dans ce meme tab connexion for read & write
	std::map<int, Client *>	tab_CGI;

	// Internal helpers

	Monitor();
	void	add_client(int fd, in_addr_t ip, in_port_t port, int fd_server);// ajoute un client  dans mon vecteur de client
	int		newClients(int i);//ajoute un nouveau socket non bloquant pour le client
	int		newRequest(int i);// RECUP la requete + verif test_read
	int		testRead(ssize_t count);
	int		disconnect(int i);//SUPPRIME LA CONNEXION SOCKET CLIENT IF !TEST_READ
	void	removeFd(int index);
	void	timeout();
	void	cleanCgi();

	// CGI handling

	int		CGI_engine(int current_fd);
	int		pollOutCgi(int i, Client *my_client);
	int		pollInCgi(int &i, Client *my_client);
	void	removeFd_CGI(Client *my_client, int y);
	void	reactivePollOut(Client *my_client, int PipeIn, int PipeOut, bool timeout);
	void	addCgiPollFd(Client *current, int i);

	// Client lifecycle

	void	afterSend(Client *current, int i, int nb_send);
	void	updateClientCookies(Client &client, const Response &resp);
	int		searchClient(Client *my_client, int i);
public :

	// Constructors / Destructor

	Monitor(std::vector<Socket *> tab);
	Monitor(const Monitor &copy);
	Monitor &operator=(const Monitor &copy);
	~Monitor();

	// Public interface

	void	addFd(int &fd, int event);//ajoute un nouveaux socket dans le tab pour poll / a faire avant le monitoring
	void	monitoring();//principal

	class MonitorError : public std::exception
	{
		const char *what() const throw();
	};
};

#endif
