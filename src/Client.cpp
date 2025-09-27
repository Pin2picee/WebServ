/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 13:41:00 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/27 17:20:43 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Client.hpp"

Client::Client() : connected(true)
{
    gettimeofday(&this->start, nullptr);
}

Client::~Client() {}

Client::Client(const Client &copy)
{
    if (this != &copy)
    {
        ip = copy.ip;
        port = copy.port;
        request = copy.request;
        reponse = copy.reponse;
        connected = copy.connected;
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
        connected = copy.connected;
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

void    Client::view_log()
{
    std::string GREEN = "\033[32m";
    std::string RESET = "\033[0m";
    std::string RED = "\033[31m";
    
    long start_h = (start.tv_sec / 3600) % 24 + 2;
    long start_m = (start.tv_sec / 60) % 60;

    long end_h = (end.tv_sec / 3600) % 24 + 2;
    long end_m = (end.tv_sec / 60) % 60;

    std::cout << RED <<"the Client " << this->ip << " connected at " << start_h << "h" << start_m;
    std::cout << " and disconnected at " << end_h << "h" << end_m << " on port: " << port << RESET << std::endl;
    std::cout << "The requete is :\n" << GREEN << request << RESET << std::endl;
}

void    Client::deconected()
{
    this->connected = false;
    gettimeofday(&this->end, nullptr);
    view_log();
}