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

/*EXCEPTION*/
const char *Socket::SocketError::what() const throw()
{
    return (strerror(errno));
}
Socket::Socket() {};


Socket::Socket(size_t port)
{
    this->_port = port;
    //creation du socket directement dans le constructeur a l'aide de la fonction socket
    //param : 1=adressIPV4, 2= TCP, 3=default TCP/ip
    this->Fd = socket(AF_INET, SOCK_STREAM, 0);
    if (Fd < 0)
        throw SocketError();
    set_socket_addr();
    if (bind(this->Fd, (const sockaddr *)&this->address1, sizeof(address1)) != 0)
        throw SocketError();
    //on lie le socket a une adress ip
}

void    Socket::set_socket_addr()
{
    this->address1.sin_family = AF_INET;// IPV4
    this->address1.sin_port = htons(this->_port);
    this->address1.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 // little_endian to big_endian for network // inet_pton mais pas le droit

}