/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/01 14:33:51 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/15 14:51:33 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP

# define SERVER_HPP

# include "defs.hpp"

class	Client;

class Server {
public:
    int port;
    std::string password;

    int epfd;
    int server_fd;
    sockaddr_in addr;
    epoll_event ev, events[MAX_EVENTS];
    
    std::map<int, Client*> clients;

    void run(char **av);
    void epoll_loop();
    
    void accept_new_client();
    void handle_client_input(int fd);
    void remove_client(int fd);

    void create_channel(const std::string &name);
    void join_channel(const std::string &channel, int fd);
    void remove_channel(const std::string &channel, int fd);
    bool look_channel(const std::string &channel, int fd);
    void broadcast_message(int sender_fd, const std::string &msg);
    bool share_channel(int fd1, int fd2);
};

int check_join_command(const std::string &msg, std::string &out_channel);

#endif
