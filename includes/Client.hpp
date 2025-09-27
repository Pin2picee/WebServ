/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 12:59:54 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/26 19:40:46 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>

class Client
{
    private :
        std::string request;
        std::string reponse ="HTTP/1.1 200 OK\r\n"
                            "Content-Length: 5\r\n"
                            "Content-Type: text/plain\r\n"
                            "\r\n"
                            "SALUT";

        std::string ip;
        std::string port;
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
        std::string    &getRequest();
        std::string    &getReponse();
};

#endif