/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:29 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/01 19:16:13 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/struct_class.hpp"
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

t_server *server_init(int port, std::string pass)
{
    if (port < 1024 || port > 65535)
      throw std::runtime_error("invalid port number, need to be between 1024 and 65535");

    t_server *serv = new t_server;
    t_rules *rules = new t_rules;

    rules->password = pass;
    rules->port = port;
    serv->rules = rules;
    serv->clients = new std::vector<Client>;
    serv->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv->server_fd == -1)
        throw std::runtime_error("socket fail\n");
    std::memset(&serv->addr, 0, sizeof(serv->addr));
    serv->addr.sin_family = AF_INET;
    serv->addr.sin_addr.s_addr = INADDR_ANY;
    serv->addr.sin_port = htons(port);


    if (bind(serv->server_fd, (sockaddr *)&serv->addr, sizeof(serv->addr)) == -1)
    {
      perror("bind fail");
      close(serv->server_fd);
      throw std::runtime_error("Bind failed — port probably already in use or requires root privileges");
    }
    if (listen(serv->server_fd, SOMAXCONN) == -1)
    {
      perror("listen fail");
      close(serv->server_fd);
      throw std::runtime_error("Listen failed");
    }


    serv->ep_fd = epoll_create1(0);
    serv->ev.events = EPOLLIN;
    serv->ev.data.fd = serv->server_fd;
    if (epoll_ctl(serv->ep_fd, EPOLL_CTL_ADD, serv->server_fd, &serv->ev) == -1)
        throw std::runtime_error("listen fail\n");
    return (serv);
}

int add_to_buffer(int client_fd, const std::string &message, t_server *server)
{
    for (size_t i = 0; i < server->clients->size(); i++)
    {
        if ((*server->clients)[i].fd != client_fd)
        {
            if ((*server->clients)[i]._output_buffer.empty())
            {
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLOUT;
                ev.data.fd = (*server->clients)[i].fd;
                if (epoll_ctl(server->ep_fd, EPOLL_CTL_MOD, (*server->clients)[i].fd, &ev) == -1)
                {
                    perror("epoll_ctl MOD fail");
                    return 1;
                }
            }
            std::string normalized = message;

            if (!normalized.empty() && normalized[normalized.size() - 1] == '\n')
            {
                normalized.erase(normalized.size() - 1);

                if (normalized.empty() || normalized[normalized.size() - 1] != '\r')
                    normalized += "\r";
                normalized += "\n";
            }
            else if (normalized.find("\r\n") == std::string::npos)
                normalized += "\r\n";
            (*server->clients)[i].appendToBuffer(normalized);
        }
    }
    return 0;
}

std::string *get_client_buff(int fd, std::vector<Client> *clients)
{
    std::vector<Client>::iterator it;

    for (it = clients->begin(); it != clients->end(); it++)
    {
        if ((*it).fd == fd)
            return (&(*it)._output_buffer);
    }
    return NULL;
}

int start_server(int port, std::string pass)
{
    t_server *serv;
    try
    {
        serv = server_init(port, pass);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
    std::cout << "server listening on Port " << serv->rules->port << std::endl;

    while (true)
    {
        int nfds = epoll_wait(serv->ep_fd, serv->events, MAX_EVENTS, -1);
        if (nfds == -1)
            throw std::runtime_error("epoll_wait fail\n");
        for (int i = 0; i < nfds; i++)
        {
            if (serv->events[i].data.fd == serv->server_fd)
            {
                int client_fd = accept(serv->server_fd, NULL, NULL);
                if (client_fd == -1)
                {
                    perror("accept fail");
                    return 1;
                }
                serv->ev.events = EPOLLIN;
                serv->ev.data.fd = client_fd;
                if (add_client(serv->ep_fd, client_fd, serv->clients, serv->ev) == -1)
                {
                    close(client_fd);
                    perror("add_client fail");
                    return 1;
                }
                char to_send[128] = "Hello!, welcome to the best IRC Server!\r\n";
                send(client_fd, to_send, strlen(to_send), 0);
            }
            else
            {
                if (serv->events[i].events & EPOLLOUT)
                {
                    std::string *buf = get_client_buff(serv->events[i].data.fd, serv->clients);
                    size_t n = (*buf).find("\r\n");
                    if (n != std::string::npos)
                    {
                        int sent = send(serv->events[i].data.fd, (*buf).c_str(), n + 2, 0);
                        if (sent == -1)
                        {
                            perror("send fail");
                            return 1;
                        }
                        else
                            (*buf).erase(0, sent);
                        if ((*buf).empty())
                        {
                            serv->ev.events = EPOLLIN;
                            serv->ev.data.fd = serv->events[i].data.fd;
                            if (epoll_ctl(serv->ep_fd, EPOLL_CTL_MOD, serv->events[i].data.fd, &serv->ev) == -1)
                            {
                                perror("epoll_ctl MOD fail");
                                return 1;
                            }
                        }
                    }
                }
                if (serv->events[i].events & EPOLLIN)
                {
                    char buf[512];
                    int n = recv(serv->events[i].data.fd, buf, sizeof(buf), 0);
                    if (n <= 0)
                    {
                        remove_client(serv->ep_fd, serv->events[i].data.fd, serv->clients);
                        std::cout << "Client disconnected!" << std::endl;
                    }
                    else
                    {
                      for (size_t j = 0; j < serv->clients->size(); j++) {
                        if ((*serv->clients)[j].fd == serv->events[i].data.fd) {
                          handle_client_input((*serv->clients)[j], std::string(buf, n));
                          break;
                        }
                      }
                    }
                }
            }
        }
    }
}
