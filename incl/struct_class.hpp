/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct_class.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:26 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/11 15:28:39 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRUCT_CLASS_HPP
# define STRUCT_CLASS_HPP

# include "defs.hpp"
# include "server.hpp"

typedef struct rules
{
	int	port;
	std::string password;
}		t_rules;

class Client {
public:
    std::string ip;
    int port;
    int fd;
    int channel;
    std::string _password;
    std::string _nickname;
    std::string _username;

    bool    nick_set;
    bool    user_set;

    std::string _input_buffer;
    std::string _output_buffer;

    Client(int fd, const std::string& ip, int port, int channel,
           const std::string& password = "",
           const std::string& nickname = "",
           const std::string& username = "");

    ~Client();
    void appendToBuffer(const std::string& message);
};

typedef struct server
{
    std::vector<Client> *clients;
    t_rules *rules;
    int server_fd;
    sockaddr_in addr;
    int ep_fd;
    epoll_event ev, events[MAX_EVENTS];
}		t_server;

#endif // !STRUCT_HPP
