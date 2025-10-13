/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:29 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/13 18:28:34 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"

/*
	fonction socket(domain, type, protocol);
	cree un socket (fd) pour le serveur
	Domain = ipv4/ipv6(1/2)
	Type = SOCK_STREAM/SOCK_DGRAM (TCP/UDP)
	Protocol = 0 (le systeme decide)

	fonction bind(sockfd, sockaddr *addr, addrlen);
	attache le socket a une adresse locale (ip:port)
	sockfd = server_fd
	addr = struct decrivant une ip
	addrlen = sizeof(addr)

	fonction listen(sockfd, backlog);
	transforme le socket en "passif" pret a recevoir des connexions
	sockfd = server fd
	backlog = nb de connections en attente

	fonction accept(sockfd, sockaddr *addr, *addrlen)
	Prend une connection presente dans la file d'attente de listen et
	cree un nouveau socket pour le client
	sockfd = server fd
	* addr = struct decrivant une ip
	addrlen = sizeof(addr)
*/

void Server::run(char **av)
{
	this->port = atoi(av[1]);
	this->password = av[2];
    
	this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->server_fd == -1)
		throw std::runtime_error("socket fail\n");
        
	std::memset(&this->addr, 0, sizeof(this->addr));
	this->addr.sin_family = AF_INET;
	this->addr.sin_addr.s_addr = INADDR_ANY;
	this->addr.sin_port = htons(port);
    
	if (bind(this->server_fd, (sockaddr *)&this->addr, sizeof(this->addr)) == -1)
	{
		close(this->server_fd);
		throw std::runtime_error("Bind failed — port probably already in use or requires root privileges");
	}

    if (listen(this->server_fd, SOMAXCONN) == -1)
    {
      close(this->server_fd);
      throw std::runtime_error("Listen failed");
    }

    this->epfd = epoll_create1(0);
    this->ev.events = EPOLLIN;
    this->ev.data.fd = this->server_fd;
    if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->server_fd, &this->ev) == -1)
        throw std::runtime_error("epoll ctl fail\n");
    
}

// t_server *server_init(int port, std::string pass)
// {
//     if (port < 1024 || port > 65535)
//       throw std::runtime_error("invalid port number,
//	need to be between 1024 and 65535");
//    
	//em gros avant 1024 il faut etre super user (sudo) donc a voir comment on bypass sa

//     t_server *serv = new t_server;
//     t_rules *rules = new t_rules;

//     rules->password = pass;
//     rules->port = port;
//     serv->rules = rules;
//     serv->server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (serv->server_fd == -1)
//         throw std::runtime_error("socket fail\n");
//     std::memset(&serv->addr, 0, sizeof(serv->addr));
//     serv->addr.sin_family = AF_INET;
//     serv->addr.sin_addr.s_addr = INADDR_ANY;
//     serv->addr.sin_port = htons(port);

//     if (bind(serv->server_fd, (sockaddr *)&serv->addr, sizeof(serv->addr)) ==
//	-1)
//     {
//       perror("bind fail");
//       close(serv->server_fd);
//       throw std::runtime_error("Bind failed — port probably already in use or requires root privileges");
//     }
//     if (listen(serv->server_fd, SOMAXCONN) == -1)
//     {
//       perror("listen fail");
//       close(serv->server_fd);
//       throw std::runtime_error("Listen failed");
//     }

//     serv->ep_fd = epoll_create1(0);
//     serv->ev.events = EPOLLIN;
//     serv->ev.data.fd = serv->server_fd;
//     if (epoll_ctl(serv->ep_fd, EPOLL_CTL_ADD, serv->server_fd, &serv->ev) ==
//	-1)
//         throw std::runtime_error("epoll ctl fail\n");
//     return (serv);
// }

// int add_to_buffer(int client_fd, const std::string &message,
	//t_server *server)
// {

// }

// std::string *get_client_buff(int fd, std::vector<Client> *clients)
// {
//     std::vector<Client>::iterator it;

//     for (it = clients->begin(); it != clients->end(); it++)
//     {
//         if ((*it).fd == fd)
//             return (&(*it)._output_buffer);
//     }
//     return (NULL);
// }

// int start_server(int port, std::string pass)
// {
//     t_server *serv;
//     try
//     {
//         serv = server_init(port, pass);
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << e.what() << '\n';
//         return (1);
//     }
//     std::cout << "server listening on Port " << serv->rules->port << std::endl;

//     while (true)
//     {
//         int nfds = epoll_wait(serv->ep_fd, serv->events, MAX_EVENTS, -1);
//         if (nfds == -1)
//             throw std::runtime_error("epoll_wait fail\n");
//         for (int i = 0; i < nfds; i++)
//         {
//             if (serv->events[i].data.fd == serv->server_fd)
//             {
//                 int client_fd = accept(serv->server_fd, NULL, NULL);
//                 if (client_fd == -1)
//                 {
//                     perror("accept fail");
//                     return (1);
//                 }
//                 serv->ev.events = EPOLLIN;
//                 serv->ev.data.fd = client_fd;
//                 if (add_client(serv, client_fd) == -1)
//                 {
//                     close(client_fd);
//                     perror("add_client fail");
//                     return (1);
//                 }
//             }
//             else
//             {
//                 if (serv->events[i].events & EPOLLIN)
//                 {
//                     char buf[512];
//                     int n = recv(serv->events[i].data.fd, buf, sizeof(buf),
		0);
//                     if (n == 0)
//                     {
//                         remove_client(serv, serv->events[i].data.fd);
//                         std::cout << "Client disconnected!" << std::endl;
//                     }
//                     else if (n > 0)
//                     {
//                         handle_client_input(*serv->clients[serv->events[i].data.fd],
//	buf);
//                     }
//                 }
//             }
//         }
//     }
// }
