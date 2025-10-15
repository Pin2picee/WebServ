/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/26 15:58:59 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/15 14:43:45 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/Monitor.hpp"

# include <csignal>

Socket *socket_a_close = nullptr;
volatile sig_atomic_t	on = 1;

void    handle_sigint(int signum)
{
    if (socket_a_close)
        close(socket_a_close->getFd());
    on = 0;
}
int main(void)
{
    std::signal(SIGINT, handle_sigint);
    ServerBlock *ptr;
    Socket  *server = new Socket(std::string("127.0.0.1"), 8080, ptr);
    
    socket_a_close = server;
    std::vector<Socket *>    all_fd;
    
    all_fd.push_back(server);

    Monitor Moniteur(all_fd);
    Moniteur.Monitoring();
    for (std::vector<Socket *>::iterator it = all_fd.begin(); it < all_fd.end(); it++)
    {
        std::cout << "Suppression d'un socket server utiliser" << std::endl;
        delete(*it);
    }
}