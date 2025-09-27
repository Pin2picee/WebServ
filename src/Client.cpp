/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 13:41:00 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/26 16:04:07 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Client.hpp"

Client::Client() {}

Client::~Client() {}

Client::Client(const Client &copy)
{
    if (this != &copy)
    {
        ip = copy.ip;
        port = copy.port;
        request = copy.request;
        reponse = copy.reponse;
    }
}

Client &Client::operator=(const Client &copy)
{
    if (this != &copy)
    {
        ip = copy.ip;
        port = copy.port;
        request = copy.request;
        reponse = copy.reponse;
    }
    return (*this);
}

void Client::setbasic(std::string ip_address, std::string port_address)
{
    ip = ip_address;
    port = port_address;
}

/*SET-GET*/

void    Client::setRequest(std::string buf)
{
    this->request = buf;
}

void    Client::setReponse(std::string buf)
{
    this->reponse = buf;
}

std::string    &Client::getRequest()
{
    return (this->request);
}

std::string &Client::getReponse()
{
    return (this->reponse);
}