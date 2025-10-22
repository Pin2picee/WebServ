/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Locations.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/09 18:08:02 by abelmoha          #+#    #+#             */
/*   Updated: 2025/10/15 15:06:12 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONS_HPP
# define LOCATIONS_HPP

# include "Webserv.hpp"

//server block va stocker les donnes d'un block server en comprenant les locations
class Locations
{
    private:
    	bool						cgi;
        bool						autoindex;
        std::string					root;
        std::string					path;
        std::string					upload_dir;
        std::string					cgi_extension;
        std::vector<std::string>	methods;
        std::vector<std::string>	index_files;
};

#endif