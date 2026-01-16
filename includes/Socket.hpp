#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Includes.hpp"

class Server;

/**
 * @brief
 * Represents a TCP listening socket.
 * 
 * @param Fd				The socket file descriptor.
 * @param address1		The socket address structure.
 * @param _port			The port the socket is bound to.
 * @param _ip			The IP address the socket listens on.
 * @param BlockServer	The associated Server configuration.
 */
class Socket
{
	private :
		int					Fd;
		struct sockaddr_in	address1;
		size_t				_port;
		std::string			_ip;
		Server				*BlockServer;

		Socket();
		void		set_socket_addr();
	public:
		Socket(std::string ip, int port, Server *ref);
		~Socket();
		Socket(const Socket &copy);
		Socket &operator=(const Socket &assignement);
	
	// Getters

		int			getFd(void) const;
		Server		*getBlockServ(void);
		size_t		getPort(void) const;
	
	// Utils

		uint32_t	ParseIp(std::string ip);
	class	SocketError : public std::exception
	{
		const char *what(void) const throw ();
	};
};

#endif
