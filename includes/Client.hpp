/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 12:59:54 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/15 15:06:45 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Webserv.hpp"
# include "Socket.hpp"

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
        Socket  *my_socket;
    public :
        Client();
        ~Client();
        Client(const Client &copy);
        Client &operator=(const Client &assignement);
        void    setbasic(std::string ip_address, std::string port_address);// assign les valeurs basic d'un nouveau client
        
    public:
        void    setRequest(std::string buf);
        int     parseRequest(void);//appelez par setReponse
        void    setReponse(std::string buf);
        void    set_socket(Socket *the_socket);
    public:
        std::string     &getRequest();
        std::string     &getReponse();
        size_t          &getOffset();
        bool            &getFinishRequest();
        Socket          *getMySocket();
    public:
        void            AddOffset(size_t nb);
        void    view_log();//affiche les temps de connexions avec l'ip et port + socket serveur
        void    deconected();//met a false + view_log()
};

#endif