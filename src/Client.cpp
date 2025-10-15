/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 13:41:00 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/15 17:09:34 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Client.hpp"

Client::Client() : connected(true)
{
    request_finish = true;
    offset = 0;
    gettimeofday(&this->start, nullptr);
}

Client::~Client()
{
    this->reponse ="HTTP/1.1 200 OK\r\n"
                            "Content-Length: 5\r\n"
                            "Content-Type: text/plain\r\n"
                            "\r\n"
                            "SALUT\r\n\r\n";
}

Client::Client(const Client &copy)
{
    if (this != &copy)
    {
        ip = copy.ip;
        port = copy.port;
        request = copy.request;
        reponse = copy.reponse;
        connected = copy.connected;
        my_socket = copy.my_socket;
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
        my_socket = copy.my_socket;
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
    this->request += buf;
    if (request.find("\r\n\r\n") != std::string::npos)
        request_finish = true;
}

void    Client::setReponse(std::string buf)
{
    this->reponse = buf;
}

std::string    &Client::getRequest()
{
    return (this->request);
}

size_t &Client::getOffset()
{
    return (this->offset);
}

bool    &Client::getFinishRequest()
{
    return (request_finish);
}

std::string &Client::getReponse()
{
    return (this->reponse);
}

Socket *Client::getMySocket()
{
    return (this->my_socket);
}

void            Client::AddOffset(size_t nb)
{
    this->offset += nb;
}

void    Client::set_socket(Socket *the_socket)
{
    this->my_socket = the_socket;
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