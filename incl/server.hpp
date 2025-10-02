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

int start_server(int port, std::string pass);
int remove_client(int epfd, int client_fd, std::vector<Client> *clients);
int add_client(int epfd, int client_fd, std::vector<Client> *clients, epoll_event ev);

#endif
