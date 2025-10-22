/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/09 16:31:13 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/15 15:06:29 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CONFIGMANAGER_HPP
# define CONFIGMANAGER_HPP

# include "Webserv.hpp"
# include "ServerBlock.hpp"

//ConfigManager = parse et charge le fichier de config

class ConfigManager
{
    private:
        std::vector<ServerBlock> BlockServer;// un vector d'objet server block
    private:
        ConfigManager();//Non instanciable sans fichier de config
        std::vector<std::string>	tokenize(std::istream &ifs);// permet de token 
    public :
        ConfigManager(const std::string &fileName);// parse + stock tous les servers block
		ConfigManager(const ConfigManager& copy);
		ConfigManager &operator=(const ConfigManager &assignement);
        ~ConfigManager();
    public:
        bool    ParseAllConfig();//parse tout les serverBlock
        bool    ParseServerBlock();//parse un server block location etc, etc puis creer les socket.
        
    class ErrorFile : public std::exception
    {
        const char *what(void) const throw();
    };
};

#endif