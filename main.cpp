/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 15:58:59 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/27 16:43:05 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Client.hpp"
#include "includes/Socket.hpp"
#include "includes/Monitor.hpp"

# include <csignal>

Socket *socket_a_close = nullptr;

void    handle_sigint(int signum)
{
    if (socket_a_close)
        close(socket_a_close->getFd());
    exit(0);
}
int main(void)
{
    std::signal(SIGINT, handle_sigint);
    Socket  server(8080);
    
    socket_a_close = &server;
    std::vector<int>    all_fd;
    
    all_fd.push_back(server.getFd());

    Monitor Moniteur(all_fd);
    Moniteur.Monitoring();
}