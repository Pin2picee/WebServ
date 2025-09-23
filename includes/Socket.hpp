/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelmoha <abelmoha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 15:22:17 by abelmoha          #+#    #+#             */
/*   Updated: 2025/09/22 15:22:17 by abelmoha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP

# define SOCKET_HPP

# include <sys/socket.h>
// #class socket va creer un socket 
class Socket
{
    private :
        int Fd;
    public:
        Socket();
        ~Socket();
        getFd();
};
#endif