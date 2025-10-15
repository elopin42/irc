#include "../incl/client.hpp"
#include "../incl/server.hpp"

int check_join_command(const std::string &msg, std::string &out_channel);

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
	channel_ok(0),
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

void Client::execute_command(const ParsedCommand &cmd)
{
	if (cmd.cmd == "PASS") {
		if (this->registered) {
			send(this->fd, "462 :You may not reregister\r\n", 30, 0);
			return;
		}
		if (cmd.args.empty()) {
			send(this->fd, "461 PASS :Not enough parameters\r\n", 34, 0);
			return;
		}
		if (cmd.args[0] == this->server->password) {
			this->pass_ok = true;
		} else {
			send(this->fd, "464 :Password incorrect\r\n", 26, 0);
		}
	}

	else if (cmd.cmd == "NICK") {
		if (cmd.args.empty()) {
			send(this->fd, "431 :No nickname given\r\n", 25, 0);
			return;
		}
		std::string newnick = cmd.args[0];

		for (std::map<int, Client*>::iterator it = this->server->clients.begin(); it != this->server->clients.end(); ++it)
		{
			Client* other = it->second;
			if (other != this && other->nickname == newnick) {
				std::string msg = "433 * " + newnick + " :Nickname is already in use\r\n";
				send(this->fd, msg.c_str(), msg.size(), 0);
				return;
			}
		}
		this->nickname = newnick;
		this->nick_ok = true;
	}

	else if (cmd.cmd == "USER") {
		if (cmd.args.size() < 4) {
			send(this->fd, "461 USER :Not enough parameters\r\n", 34, 0);
			return;
		}
		this->username = cmd.args[0];
		this->realname = cmd.args[3];
		this->user_ok = true;
	}

	else if (cmd.cmd == "PING") {
		if (cmd.args.empty())
			send(this->fd, "409 :No origin specified\r\n", 27, 0);
		else {
			std::string pong = "PONG " + cmd.args[0] + "\r\n";
			send(this->fd, pong.c_str(), pong.size(), 0);
		}
	}

	else if (cmd.cmd == "JOIN") {
		if (cmd.args.empty()) {
			send(this->fd, "461 JOIN :Not enough parameters\r\n", 34, 0);
			return;
		}
		std::string chan = cmd.args[0];
		this->server->join_channel(chan, this->fd);
	}

	else if (cmd.cmd == "PRIVMSG") {
		if (cmd.args.size() < 2) {
			send(this->fd, "461 PRIVMSG :Not enough parameters\r\n", 37, 0);
			return;
		}
		std::string target = cmd.args[0];
		std::string message = cmd.args[1];
		std::string formatted = ":" + this->nickname + " PRIVMSG " + target + " :" + message + "\r\n";
		this->server->broadcast_message(this->fd, formatted);
	}

	else if (cmd.cmd == "QUIT") {
		std::string msg = cmd.args.empty() ? "Client quit" : cmd.args[0];
		std::cout << "[QUIT] " << this->fd << ": " << msg << std::endl;
		this->server->remove_client(this->fd);
	}

	else {
		std::string err = "421 " + cmd.cmd + " :Unknown command\r\n";
		send(this->fd, err.c_str(), err.size(), 0);
	}

	if (this->pass_ok && this->nick_ok && this->user_ok && !this->registered) {
		this->registered = true;
		std::string msg = "001 " + this->nickname + " :Welcome to the IRC server!\r\n";
		send(this->fd, msg.c_str(), msg.size(), 0);
	}
}


void Client::parse_lines()
{
	while (!this->lines_to_parse.empty()) {
		std::string line = this->lines_to_parse.front();
		this->lines_to_parse.erase(this->lines_to_parse.begin());

		if (!line.empty() && line[line.size() - 1] == '\n')
    		line.erase(line.size() - 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);


		std::istringstream iss(line);
		ParsedCommand cmd;
		iss >> cmd.cmd;

		std::string token;
		bool in_trailing = false;
		while (iss >> token) {
			if (token[0] == ':' && !in_trailing) {
				std::string trailing = token.substr(1);
				std::string rest;
				std::getline(iss, rest);
				trailing += rest;
				cmd.args.push_back(trailing);
				in_trailing = true;
			}
			else if (!in_trailing) {
				cmd.args.push_back(token);
			}
		}

		std::transform(cmd.cmd.begin(), cmd.cmd.end(), cmd.cmd.begin(), ::toupper);

		this->execute_command(cmd);
	}
}


// void Client::parse_lines()
// {
// 	//Travail de callista (parser les lines detecter les commandes et mettre dans un autre vector de commande a executer faire en sorte que les strings de ce vector soit simple a executer et claire, une structure serait meilleure) struct : le fd du sender, le mot de la cmd, ses args, et pour l'istat le "goat" ne sait pas car nul
// 	std::vector<std::string>::iterator it = this->lines_to_parse.begin();
// 	while (it != this->lines_to_parse.end())
// 	{
// 		// Broadcast temporaire
// 		for (std::map<int, Client*>::iterator client_it = this->server->clients.begin(); client_it != this->server->clients.end(); ++client_it)
// 		{
// 			if (client_it->second != this)
// 			{
// 				bool was_empty = client_it->second->send_buf.empty();
// 				client_it->second->send_buf.push_back(*it);
// 				if (was_empty)
// 				{
// 					this->server->ev.events = EPOLLIN | EPOLLOUT;
// 					this->server->ev.data.fd = client_it->first;
// 					if (epoll_ctl(this->server->epfd, EPOLL_CTL_MOD, client_it->first, &this->server->ev) == -1)
// 						throw std::runtime_error("epoll_ctl fail");
// 				}
// 			}
// 		}
// 		it = this->lines_to_parse.erase(it);
// 	}
// }

void Client::send_pending()
{
	std::vector<std::string>::iterator it = this->send_buf.begin();
	if (it != this->send_buf.end())
	{
		std::string channel;
		int ch = check_join_command(*it, channel);
		if (ch != -1)
		{
			// if (this->server->look_channel(channel, this->fd))
			// {
			//     this->server->remove_channel(channel, this->fd);
			//     std::cout << "[ACTION] Client " << this->fd
			//               << " left channel " << channel << std::endl;
			// } // em gros sa sera utile pour supprimer un channel dedier sa
				this->server->join_channel(channel, this->fd);
		}
		else
			this->server->broadcast_message(this->fd, *it);
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

