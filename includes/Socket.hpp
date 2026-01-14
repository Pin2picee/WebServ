#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Includes.hpp"

class Server;

// #class qui va creer un socket 
class Socket
{
	private :
		int					Fd;// file descriptor genere par socket()
		struct sockaddr_in	address1;// structure pour paramtrer l'adrresse a bind
		size_t				_port;
		std::string			_ip;
		Server				*BlockServer;
	private :
		void		set_socket_addr();//methode qui definie les valeurs a implement
	public:
		Socket(std::string ip, int port, Server *ref);
		~Socket();// le ferme
		Socket(const Socket &copy);
		Socket &operator=(const Socket &assignement);
	public:
		int			getFd(void) const;// recupere le
		Server		*getBlockServ(void);//donne une reference a son Server 
	public:
		uint32_t	ParseIp(std::string ip);
	class   SocketError : public std::exception
	{
		const char *what(void) const throw ();
	};
};

#endif