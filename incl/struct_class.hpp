/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct_class.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:26 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/01 15:51:23 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRUCT_CLASS_HPP
# define STRUCT_CLASS_HPP

# include <iostream>
# include <string>
# include "server.hpp"

typedef struct rules
{
	int	port;
	std::string password;
}		t_rules;

typedef struct server
{
}		t_server;

class Client {
public:
    int fd;
    std::string ip;
    int port;
    std::string _password;
    std::string _nickname;
    std::string _username;

    Client(int fd, const std::string& ip, int port,
           const std::string& password = "",
           const std::string& nickname = "",
           const std::string& username = "");

    ~Client();
};



#endif // !STRUCT_HPP
