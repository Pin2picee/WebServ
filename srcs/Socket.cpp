/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 15:24:53 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/22 15:24:53 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Socket.hpp"

/*<EXCEPTION>*/
const char *Socket::SocketError::what() const throw()
{
    return (strerror(errno));
}
/*</EXCEPTION>*/


/*<CONSTRUCTION>*/
Socket::Socket() {};

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
Socket::Socket(std::string ip, int port, ServerBlock *refBlock)
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
    else if (listen(this->Fd, 10000) < 0)// 2emeparam= backlog file d'attente dont la connexion n'est pas encore accepter
        throw SocketError();
    int ancien_flags = fcntl(this->Fd, F_GETFL);
    fcntl(this->Fd, ancien_flags | O_NONBLOCK);
}
/*</CONSTRUCTION>*/

/*<DESTRUCTION>*/
Socket::~Socket()
{
    close(this->Fd);
}
/*</DESTRUCTION>*/

uint32_t Socket::ParseIp(std::string ip)
{
    std::stringstream iss(ip);
    int a, b, c, d;
    char point;
    uint32_t res;

    iss >> a >> point >> b >> point >> c >> point >> d;
    res = a << 24 | b << 16 | c << 8 | d;
    return (res);
}

void    Socket::set_socket_addr()
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

ServerBlock *Socket::getBlockServ(void)
{
    return (this->BlockServer);
}