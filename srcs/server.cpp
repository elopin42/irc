/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:29 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/14 14:08:57 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

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
	char buf[512];
	int n = recv(fd, buf, sizeof(buf), 0);
	if (n == -1)
		throw std::runtime_error("recv error");
	else if (n == 0)
		this->remove_client(fd);
	else if (n > 0)
	{
		this->clients[fd]->recv_buf.append(buf, n);
		this->clients[fd]->process_data();
	}
}

void Server::epoll_loop()
{
	while (true)
	{
		int nfds = epoll_wait(this->epfd, this->events, MAX_EVENTS, -1);
		if (nfds == -1)
			throw std::runtime_error("epoll_wait fail\n");
		for (int i = 0; i < nfds; i++)
		{
			if (this->events[i].data.fd == this->server_fd)
				this->accept_new_client();
			else
			{
				if (this->events[i].events & EPOLLIN)
					this->handle_client_input(this->events[i].data.fd);
				else if (this->events[i].events & EPOLLOUT)
					this->clients[this->events[i].data.fd]->send_pending();
			}
		}
	}
}
