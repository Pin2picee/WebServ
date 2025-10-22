#ifndef MONITOR_HPP

# define MONITOR_HPP

# include "Client.hpp"
# include "Socket.hpp"

extern volatile sig_atomic_t	on;

/**
 * @brief = FUTUR instance qui va permettre de poll mes socket serveurs et client car on a le droit a un seul poll
 * @example = si monitoring lancer on ne add_fd plus et on ne rajoute plus de socket server
 */

class Monitor
{
	private :
		size_t				nb_fd;
		size_t				nb_fd_server;
		std::map<int, Client>	clients;
		std::map<int, Socket *>	all_socket;//ma map avec cle = fd, valeur = obj Socket;
		struct  pollfd		all_fd[200000];// dois tout avoir dans ce meme tab connexion for read & write
	private :
		Monitor();
		void 	add_client(int fd, in_addr_t ip, in_port_t port, int fd_server);// ajoute un client  dans mon vecteur de client
		int		new_clients(int i);//ajoute un nouveau socket non bloquant pour le client
		int		new_request(int i);// RECUP la requete + verif test_read
		int		test_read(ssize_t count);
		int		deconnexion(int i);//SUPPRIME LA CONNEXION SOCKET CLIENT IF !TEST_READ
	public :
		Monitor(std::vector<Socket *> tab);
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