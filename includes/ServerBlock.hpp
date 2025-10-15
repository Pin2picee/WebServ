/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/09 18:08:00 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/15 15:57:59 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

# include "Webserv.hpp"

//server block va stocker les donnes d'un block server en comprenant les locations
class ServerBlock
{
    private:
        std::vector<std::pair<std::string, int> >   listen;
        std::string								    root;
        std::map<int, std::string>				    error_pages;
        size_t									    client_max_body_size;
        std::vector <Locations *>                     locationsServer;//toutes les locations
    private:
        ServerBlock();
    public:
        ServerBlock(const std::string &BlocServer);
        ServerBlock(const ServerBlock &copy);
        ServerBlock &operator=(const ServerBlock &assignement);
        ~ServerBlock();
};

#endif