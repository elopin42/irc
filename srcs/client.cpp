#include "../incl/client.hpp"
#include "../incl/server.hpp"

Client::Client(int fd, Server *serv)
    : server(serv),
      fd(fd),
      nickname(),
      username(),
      realname(),
      temp_pass(""),
      pass_ok(0),
      nick_ok(0),
      user_ok(0),
      registered(0),
      kick(0),
      first_try(1),
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
        if (pos + 1 > 512)
        {
            std::cerr << "[WARN] Client " << this->fd << " sent message over 512 bytes, ignoring\r\n";
            this->recv_buf = this->recv_buf.substr(pos + 1);
            pos = 0;
            continue;
        }
        
        if (pos > 0 && this->recv_buf[pos - 1] != '\r')
        {
            this->recv_buf.insert(pos, 1, '\r');
            pos++;
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
    while (!this->lines_to_parse.empty())
    {
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
        while (iss >> token)
        {
            if (token[0] == ':' && !in_trailing)
            {
                std::string trailing = token.substr(1);
                std::string rest;
                std::getline(iss, rest);
                trailing += rest;
                cmd.args.push_back(trailing);
                in_trailing = true;
            }
            else if (!in_trailing)
            {
                cmd.args.push_back(token);
            }
        }
        cmd.fd = this->fd;
        this->server->execute_command(cmd);
    }
}

void Client::add_to_send_buf(const std::string &data)
{
    std::string msg = data;
    
    // IRC messages must not exceed 512 bytes (including CRLF)
    if (msg.size() > 512)
    {
        // Truncate to 510 bytes to leave room for CRLF
        msg = msg.substr(0, 510);
        
        // Make sure we don't cut in the middle of a CRLF
        if (msg.size() >= 2 && msg.substr(msg.size() - 2) == "\r\n")
        {
            msg = msg.substr(0, msg.size() - 2);
        }
        else if (msg.size() >= 1 && (msg[msg.size() - 1] == '\r' || msg[msg.size() - 1] == '\n'))
        {
            msg = msg.substr(0, msg.size() - 1);
        }
        
        msg += "\r\n";
    }
    
    bool was_empty = this->send_buf.empty();
    if (was_empty)
    {
        this->server->ev.events = EPOLLIN | EPOLLOUT;
        this->server->ev.data.fd = this->fd;
        if (epoll_ctl(this->server->epfd, EPOLL_CTL_MOD, this->fd, &this->server->ev) == -1)
            throw std::runtime_error("epoll_ctl fail");
    }
    this->send_buf.push_back(msg);
}

void Client::send_pending()
{
    std::vector<std::string>::iterator it = this->send_buf.begin();
    if (it != this->send_buf.end())
    {
        ssize_t sent = send(this->fd, (*it).c_str(), (*it).size(), 0);
        if (sent > 0)
            it = this->send_buf.erase(it);
        if (this->send_buf.empty())
        {
            if (this->kick)
                this->server->remove_client(this->fd);
            else
            {
                this->server->ev.events = EPOLLIN;
                this->server->ev.data.fd = this->fd;
                if (epoll_ctl(this->server->epfd, EPOLL_CTL_MOD, this->fd, &this->server->ev) == -1)
                    throw std::runtime_error("epoll_ctl fail");
            }
        }
    }
}
