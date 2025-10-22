/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:29 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/22 23:18:15 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"
#include <csignal>
#include <cerrno>

extern bool g_running;

Server::~Server()
{
    delete_all(this);
}

Client *Server::find_client_by_nickname(const std::string &nickname)
{
    for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        if (it->second->nickname == nickname)
            return it->second;
    }
    return NULL;
}

void Server::send_to(int fd, std::string to_send)
{
    std::map<int, Client *>::iterator it = this->clients.find(fd);
    if (it != this->clients.end() && it->second)
        it->second->add_to_send_buf(to_send);
}

int Server::resolve_user_fd(const std::string &user)
{
    std::map<int, Client *>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        if (this->clients[it->first]->nickname == user)
            return it->first;
    }
    return -1;
}

void Server::create_channel(const std::string &name)
{
    this->channels[name] = new Channel(name, this);
    std::cout << "[INFO] created channel " << name << std::endl;
}

void Server::initialize_handled_commands()
{
    this->handled_commands.push_back("PASS");
    this->handled_commands.push_back("NICK");
    this->handled_commands.push_back("USER");
    this->handled_commands.push_back("QUIT");
    this->handled_commands.push_back("PING");
    this->handled_commands.push_back("PONG");
    this->handled_commands.push_back("CAP");
    this->handled_commands.push_back("PRIVMSG");
    this->handled_commands.push_back("NOTICE");
    this->handled_commands.push_back("JOIN");
    this->handled_commands.push_back("PART");
    this->handled_commands.push_back("TOPIC");
    this->handled_commands.push_back("MODE");
    this->handled_commands.push_back("INVITE");
    this->handled_commands.push_back("KICK");
    this->handled_commands.push_back("WHO");
    this->handled_commands.push_back("WHOIS");
    this->handled_commands.push_back("NAMES");
    this->handled_commands.push_back("LIST");
    this->handled_commands.push_back("ERROR");

    this->initialize_command_map();
}

void Server::execute_command(const ParsedCommand &cmd)
{
    if (this->command_map.find(cmd.cmd) != this->command_map.end())
    {
        (this->*command_map[cmd.cmd])(cmd);
        return;
    }

    std::cout << "[DEBUG] Command not recognized " << cmd.cmd << std::endl;
    std::string err = ":irc.local 421 " + cmd.cmd + " :Command not implemented\r\n";
    this->send_to(cmd.fd, err);
}

void Server::initialize_command_map()
{
    this->command_map["PING"] = &Server::PING;
    this->command_map["PONG"] = &Server::PONG;
    this->command_map["PRIVMSG"] = &Server::PRIVMSG;
    this->command_map["JOIN"] = &Server::JOIN;
    this->command_map["CAP"] = &Server::CAP;
    this->command_map["PASS"] = &Server::PASS;
    this->command_map["NICK"] = &Server::NICK;
    this->command_map["USER"] = &Server::USER;
    this->command_map["QUIT"] = &Server::QUIT;
    this->command_map["NOTICE"] = &Server::NOTICE;
    this->command_map["PART"] = &Server::PART;
    this->command_map["TOPIC"] = &Server::TOPIC;
    this->command_map["MODE"] = &Server::MODE;
    this->command_map["INVITE"] = &Server::INVITE;
    this->command_map["KICK"] = &Server::KICK;
    this->command_map["WHO"] = &Server::WHO;
    this->command_map["WHOIS"] = &Server::WHOIS;
}

void Server::remove_channel(const std::string &channel)
{
    std::map<std::string, Channel *>::iterator it = this->channels.find(channel);

    if (it != this->channels.end())
    {
        std::string channel_name = it->second->name;
        delete it->second;
        this->channels.erase(it);
        std::cout << "[INFO] Channel " << channel_name << " removed from map" << std::endl;
    }
    else
    {
        std::cout << "[WARN] Channel " << channel << " not found in map" << std::endl;
    }
}

void Server::accept_new_client()
{
    int client_fd = accept(this->server_fd, NULL, NULL);
    if (client_fd == -1)
        throw std::runtime_error("accept fail");

    this->ev.events = EPOLLIN;
    this->ev.data.fd = client_fd;
    if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, client_fd, &this->ev) == -1)
    {
        close(client_fd);
        throw std::runtime_error("epoll_ctl fail");
    }
    this->clients[client_fd] = new Client(client_fd, this);
    std::cout << "[INFO] New client added (fd=" << client_fd << ")" << std::endl;
}

void Server::remove_client(int fd)
{
    std::map<int, Client *>::iterator it = this->clients.find(fd);

    if (it != this->clients.end())
    {
        delete it->second;
        this->clients.erase(it);
    }

    if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, fd, NULL) == -1)
        throw std::runtime_error("epoll ctl fail");

    close(fd);
    std::cout << "[INFO] Client removed (fd=" << fd << ")" << std::endl;
}

void Server::run(char **av)
{
    this->port = atoi(av[1]);
    this->password = av[2];
    this->initialize_handled_commands();

    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_fd == -1)
        throw std::runtime_error("socket fail\n");

    int opt = 1;
    if (setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt fail\n");

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

    std::cout << "server listening on Port " << this->port << std::endl;
    this->epoll_loop();
}

void Server::handle_client_input(int fd)
{
    const size_t RECV_BUFFER_SIZE = 4096;
    char buf[RECV_BUFFER_SIZE];
    int n = recv(fd, buf, RECV_BUFFER_SIZE - 1, 0);
    if (n == -1)
        throw std::runtime_error("recv error");
    else if (n == 0)
        this->remove_client(fd);
    else if (n > 0)
    {
        std::map<int, Client *>::iterator it = this->clients.find(fd);
        if (it == this->clients.end())
            return;
        
        // Prevent recv_buf from growing too large (max 64KB per client)
        if (it->second->recv_buf.size() + n > 65536)
        {
            std::cerr << "[WARN] Client " << fd << " buffer overflow attempt, disconnecting" << std::endl;
            this->remove_client(fd);
            return;
        }
        it->second->recv_buf.append(buf, n);
        it->second->process_data();
    }
}

void Server::epoll_loop()
{
    while (g_running)
    {
        int nfds = epoll_wait(this->epfd, this->events, MAX_EVENTS, 1000); // timeout 1s pour réagir au Ctrl+C
        if (nfds == -1)
        {
            if (errno == EINTR)
                continue; // interruption normale (Ctrl+C)
            throw std::runtime_error("epoll_wait fail");
        }

        for (int i = 0; i < nfds; i++)
        {
            int fd = this->events[i].data.fd;
            if (fd == this->server_fd)
                this->accept_new_client();
            else
            {
                if (this->clients.find(fd) == this->clients.end())
                    continue;
                    
                if (this->events[i].events & EPOLLIN)
                    this->handle_client_input(fd);
                    
                if (this->clients.find(fd) == this->clients.end())
                    continue;
                    
                if (this->events[i].events & EPOLLOUT)
                {
                    Client *cli = this->clients[fd];
                    if (cli)
                        cli->send_pending();
                }
            }
        }
    }

    std::cout << "[INFO] Exiting epoll loop, cleaning up..." << std::endl;

    if (this->server_fd > 0)
        close(this->server_fd);

    delete_all(this);
}
