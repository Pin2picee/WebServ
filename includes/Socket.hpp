#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Includes.hpp"

class Server;

// #class qui va creer un socket 
class Socket
{
	private :
		int					Fd;
		struct sockaddr_in	address1;
		size_t				_port;
		std::string			_ip;
		Server				*BlockServer;
	private :
		Socket();
		void		set_socket_addr();
	public:
		Socket(std::string ip, int port, Server *ref);
		~Socket();
		Socket(const Socket &copy);
		Socket &operator=(const Socket &assignement);
	public:
		int			getFd(void) const;
		Server		*getBlockServ(void);
		size_t		getPort(void) const;
	public:
		uint32_t	ParseIp(std::string ip);
	class   SocketError : public std::exception
	{
		const char *what(void) const throw ();
	};
};

#endif
