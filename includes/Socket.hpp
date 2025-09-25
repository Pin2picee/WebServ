/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 15:22:17 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/22 15:22:17 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP

# define SOCKET_HPP

# include <sys/socket.h>   // pour socket(), bind(), sockaddr et macro
# include <netinet/in.h>   // pour sockaddr_in, in_addr
# include <iostream>
# include <errno.h>
# include <string.h>
# includee <unistd.h>
// #class qui va creer un socket 
class Socket
{
    private :
        int Fd;// file descriptor genere par socket()
        struct sockaddr_in address1;// structure pour paramtrer l'adrresse a bind
        size_t _port;
        
    private :
        Socket();//lance un socket
        void    set_socket_addr();//methode qui definie les valeurs a implement
    public:
        Socket(size_t port);
        ~Socket();// le ferme
        int getFd(void) const;// recupere le 

    class   SocketError : public std::exception
    {
        const char *what(void) const throw ();
    };
};
#endif