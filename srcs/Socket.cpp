/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/23 20:34:22 by abelmoha          #+#    #+#             */
/*   Updated: 2025/12/23 20:34:22 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

/**
 * @brief
 * Custom exception class for socket-related errors.
 */
const char *Socket::SocketError::what() const throw()
{
	return (strerror(errno));
}

/**
 * @brief
 * Copy constructor for Socket.
 *
 * @param copy The Socket object to copy from.
 */
Socket::Socket(const Socket &copy)
{
	if (this != &copy)
	{
		Fd = copy.Fd;
		address1 = copy.address1;
		_port = copy._port;
		BlockServer = copy.BlockServer;
	}
}

/**
 * @brief
 * Assignment operator for Socket.
 *
 * @param assignement The Socket object to assign from.
 *
 * @return Reference to the current Socket object.
 */
Socket &Socket::operator=(const Socket &assignement)
{
	if (this != &assignement)
	{
		Fd = assignement.Fd;
		address1 = assignement.address1;
		_port = assignement._port;
		BlockServer = assignement.BlockServer;
	}
	return (*this);
}

/**
 * @brief
 * Create a socket, bind it to the given IP and port, and set it to listen mode.
 *
 * @param ip IP address to bind the socket to.
 * @param port Port number to listen on (must be 1-65535).
 * @param refBlock Pointer to the Server object associated with this socket.
 *
 * @throws SocketError if socket creation, bind, listen, or fcntl fails.
 */
Socket::Socket(std::string ip, int port, Server *refBlock)
{
	this->_port = port;
	this->_ip = ip;
	this->BlockServer = refBlock;
	this->Fd = socket(AF_INET, SOCK_STREAM, 0);
	if (Fd < 0 || port <= 0 || port > 65535)
		throw SocketError();
	set_socket_addr();
	if (bind(this->Fd, (const sockaddr *)&this->address1, sizeof(address1)) != 0)
		throw SocketError();
	else if (listen(this->Fd, SOMAXCONN) < 0)
		throw SocketError();
	int ancien_flags = fcntl(this->Fd, F_GETFL);
	int res = fcntl(this->Fd, F_SETFL, ancien_flags | O_NONBLOCK);
	if (ancien_flags == -1 || res == -1)
		throw SocketError();
}

/**
 * @brief
 * Default constructor for Socket.
 */
Socket::Socket() {}

/**
 * @brief
 * Destructor for Socket. Closes the file descriptor.
 */
Socket::~Socket()
{
	close(this->Fd);
}

/**
 * @brief
 * Convert an IP string (e.g., "127.0.0.1") to a 32-bit integer.
 *
 * @param ip The IP address string.
 *
 * @return The 32-bit representation of the IP, or 0 if invalid.
 */
uint32_t Socket::ParseIp(std::string ip)
{
	std::stringstream ss(ip);
	int a, b, c, d;
	char dot1, dot2, dot3;

	if (!(ss >> a >> dot1 >> b >> dot2 >> c >> dot3 >> d))
		return 0;

	if (dot1 != '.' || dot2 != '.' || dot3 != '.')
		return 0;

	if (a < 0 || a > 255 || b < 0 || b > 255
		|| c < 0 || c > 255 || d < 0 || d > 255)
		return 0;

	char extra;
	if (ss >> extra)
		return 0;

	uint32_t res =
		(static_cast<uint32_t>(a) << 24) |
		(static_cast<uint32_t>(b) << 16) |
		(static_cast<uint32_t>(c) << 8 ) |
		(static_cast<uint32_t>(d));

	return res;
}

/**
 * @brief
 * Set the socket address structure and configure socket options (SO_REUSEADDR).
 */
void	Socket::set_socket_addr()
{
	int option = 1;
	setsockopt(this->Fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	this->address1.sin_family = AF_INET;
	this->address1.sin_port = htons(this->_port);
	this->address1.sin_addr.s_addr = htonl(this->ParseIp(_ip));
}

/**
 * @brief
 * Get the file descriptor of the socket.
 *
 * @return The socket's file descriptor.
 */
int Socket::getFd(void) const
{
	return (this->Fd);
}

/**
 * @brief
 * Get the Server object associated with this socket.
 *
 * @return Pointer to the Server object.
 */
Server *Socket::getBlockServ(void)
{
	return (this->BlockServer);
}

/**
 * @brief
 * Get the port number the socket is bound to.
 *
 * @return The port number.
 */
size_t	Socket::getPort(void) const
{
	return (this->_port);
}
