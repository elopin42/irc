/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/01 14:33:51 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/14 12:46:48 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP

# define SERVER_HPP

# include "defs.hpp"

class	Client;

class   Channel;

class Server {
public:
    int port;
    std::string password;

    int epfd;
    int server_fd;
    sockaddr_in addr;
    epoll_event ev, events[MAX_EVENTS];
    
    std::map<int, Client*> clients;
    std::map<std::string, Channel*> channels;

    void run(char **av);
    void epoll_loop();
    
    void accept_new_client();
    void handle_client_input(int fd);
    void remove_client(int fd);
};

#endif
