/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/01 14:33:51 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/01 19:16:20 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP

#define SERVER_HPP

#include "defs.hpp"
#include "struct_class.hpp"

class Client;

typedef struct server t_server;

int start_server(int port, std::string pass);
int remove_client(int epfd, int client_fd, std::vector<Client> *clients);
int add_client(int epfd, int client_fd, std::vector<Client> *clients, epoll_event ev);
void handle_client_input(Client &client, const std::string &data);
Client find_client_by_fd(const std::map<int, std::vector<Client> > &channels, int fd_to_find);
int find_channel_by_fd(const std::map<int, std::vector<Client> > &channels, int fd_to_find);
void broadcast_message(std::vector<Client> *clients, int sender_fd, const std::string &msg);
int add_client(int epfd, int client_fd, std::vector<Client> *clients, epoll_event ev, server *serv);

#endif
