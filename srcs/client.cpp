#include "../incl/client.hpp"

Client::Client(int fd, Server* serv)
	: server(serv),
	  fd(fd),
	  nickname(),
	  username(),
	  realname(),
	  pass_ok(0),
	  nick_ok(0),
	  user_ok(0),
	  registered(0),
	  lines_to_parse(),
	  recv_buf(),
	  send_buf(),
	  joined_channels() {}

Client::~Client() {}

void Client::process_data()
{
	size_t pos = 0;
	while ((pos = this->recv_buf.find('\n', pos)) != std::string::npos)
	{
		if (pos > 0 && this->recv_buf[pos - 1] != '\r')
		{
			this->recv_buf.insert(pos, 1, '\r');
			pos++; // adjust pos since we inserted
		}
		std::string line = this->recv_buf.substr(0, pos + 1);
		lines_to_parse.push_back(line);
		this->recv_buf = this->recv_buf.substr(pos + 1);
		pos = 0;
	}
	this->parse_lines();
}

void Client::parse_lines()
{
	//Travail de callista (parser les lines detecter les commandes et mettre dans un autre vector de commande a executer faire en sorte que les strings de ce vector soit simple a executer et claire, une structure serait meilleure)
	std::vector<std::string>::iterator it = this->lines_to_parse.begin();
	while (it != this->lines_to_parse.end())
	{
		// Broadcast temporaire
		for (std::map<int, Client*>::iterator client_it = this->server->clients.begin(); client_it != this->server->clients.end(); ++client_it)
		{
			if (client_it->second != this)
			{
				bool was_empty = client_it->second->send_buf.empty();
				client_it->second->send_buf.push_back(*it);
				if (was_empty)
				{
					this->server->ev.events = EPOLLIN | EPOLLOUT;
					this->server->ev.data.fd = client_it->first;
					if (epoll_ctl(this->server->epfd, EPOLL_CTL_MOD, client_it->first, &this->server->ev) == -1)
						throw std::runtime_error("epoll_ctl fail");
				}
			}
		}
		it = this->lines_to_parse.erase(it);
	}
}

void Client::send_pending()
{
	std::vector<std::string>::iterator it = this->send_buf.begin();
	if (it != this->send_buf.end())
	{
		send(this->fd, (*it).c_str(), (*it).size(), 0);
		it = this->send_buf.erase(it);
		if (this->send_buf.empty())
		{
			this->server->ev.events = EPOLLIN;
			this->server->ev.data.fd = this->fd;
			if (epoll_ctl(this->server->epfd, EPOLL_CTL_MOD, this->fd, &this->server->ev) == -1)
				throw std::runtime_error("epoll_ctl fail");
		}
	}
}