/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 12:59:54 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/01 21:14:05 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include <sys/time.h>

class Client
{
    private :
        std::string request;
        std::string reponse ="HTTP/1.1 200 OK\r\n"
                            "Content-Length: 5\r\n"
                            "Content-Type: text/plain\r\n"
                            "\r\n"
                            "SALUT\r\n\r\n";

        std::string ip;
        std::string port;
        timeval start;
        timeval end;
        bool    connected;
        bool    request_finish;
        size_t  offset;
    public :
        Client();
        ~Client();
        Client(const Client &copy);
        Client &operator=(const Client &assignement);
        void    setbasic(std::string ip_address, std::string port_address);// assign les valeurs basic d'un nouveau client
        
    public:
        void    setRequest(std::string buf);
        void    setReponse(std::string buf);
    public:
        std::string     &getRequest();
        std::string     &getReponse();
        size_t          &getOffset();
        bool            &getFinishRequest();
        void            AddOffset(size_t nb);
    public:
        void    view_log();//affiche les temps de connexions avec l'ip et port + socket serveur
        void    deconected();//met a false + view_log()
};

#endif