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

/*<EXCEPTION>*/
const char *Socket::SocketError::what() const throw()
{
	return (strerror(errno));
}
/*</EXCEPTION>*/

/*<CONSTRUCTION>*/
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
 * @brief = creation du socket + liaison(=bind) localhost + port
 * @param = port sur lequel on va listen
 */
Socket::Socket(std::string ip, int port, Server *refBlock)
{
	this->_port = port;
	this->_ip = ip;
	this->BlockServer = refBlock;
	this->Fd = socket(AF_INET, SOCK_STREAM, 0);
	if (Fd < 0 || port <= 0 || port > 65535)
		throw SocketError();
	set_socket_addr();// initialisation de la structure sock_addr_in puis convertit en (const sock_adrr *)
	if (bind(this->Fd, (const sockaddr *)&this->address1, sizeof(address1)) != 0)
		throw SocketError();
	else if (listen(this->Fd, SOMAXCONN) < 0)// 2emeparam= backlog file d'attente dont la connexion n'est pas encore accepter
		throw SocketError();
	int ancien_flags = fcntl(this->Fd, F_GETFL);
	fcntl(this->Fd, ancien_flags | O_NONBLOCK);
}
/*</CONSTRUCTION>*/

/*<DESTRUCTION>*/
Socket::Socket() {}

Socket::~Socket()
{
	close(this->Fd);
}
/*</DESTRUCTION>*/

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

void	Socket::set_socket_addr()
{
	int option = 1;
	setsockopt(this->Fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	this->address1.sin_family = AF_INET;
	this->address1.sin_port = htons(this->_port);
	this->address1.sin_addr.s_addr = htonl(this->ParseIp(_ip)); // 127.0.0.1 // little_endian to big_endian for network // inet_pton->function mais pas le droit
}

int Socket::getFd(void) const
{
	return (this->Fd);
}

Server *Socket::getBlockServ(void)
{
	return (this->BlockServer);
}