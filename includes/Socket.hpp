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

# include "Webserv.hpp"
# include "ConfigManager.hpp"

// #class qui va creer un socket 
class Socket
{
    private :
        int Fd;// file descriptor genere par socket()
        struct sockaddr_in address1;// structure pour paramtrer l'adrresse a bind
        size_t _port;
        std::string _ip;
        ServerBlock *BlockServer;
    private :
        Socket();//lance un socket
        void    set_socket_addr();//methode qui definie les valeurs a implement
    public:
        Socket(std::string ip, int port, ServerBlock *ref);
        ~Socket();// le ferme
        Socket(const Socket &copy);
        Socket &operator=(const Socket &assignement);
    public:
        int getFd(void) const;// recupere le
        ServerBlock *getBlockServ(void);//donne une reference a son serverBlock 
    public:
        uint32_t    ParseIp(std::string ip);
    class   SocketError : public std::exception
    {
        const char *what(void) const throw ();
    };
};
#endif