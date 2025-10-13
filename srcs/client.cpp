#include "../incl/server.hpp"

int	add_client(t_server *serv, int client_fd)
{
	if (epoll_ctl(serv->ep_fd, EPOLL_CTL_ADD, client_fd, &serv->ev) == -1)
	{
		perror("epoll_ctl ADD client");
		return (-1);
	}
	serv->clients[client_fd] = new Client(client_fd);
	std::cout << "[INFO] New client added (fd=" << client_fd << ")" << std::endl;
	return (0);
}

int	remove_client(t_server *serv, int client_fd)
{
	if (epoll_ctl(serv->ep_fd, EPOLL_CTL_DEL, client_fd, NULL) == -1)
	{
		perror("epoll_ctl DEL client");
		return (-1);
	}

	close(client_fd);

	std::unordered_map<int, Client *>::iterator it = serv->clients.find(client_fd);
	if (it != serv->clients.end())
    {
		delete it->second;
        serv->clients.erase(it);
    }

	std::cout << "[INFO] Client removed (fd=" << client_fd << ")" << std::endl;
	return (0);
}

void Client::appendToBuffer(const std::string &message)
{
	_input_buffer += message;
}

int	handle_client_input(Client &client, const std::string &data)
{
    client.appendToBuffer(data);
	
}

void	broadcast_message(std::vector<Client> *clients, int sender_fd,
		const std::string &msg)
{
	int	client_fd;

	std::stringstream ss;
	ss << "[Client " << sender_fd << "] " << msg;
	std::string final_msg = ss.str();
	for (size_t i = 0; i < clients->size(); i++)
	{
		client_fd = (*clients)[i].fd;
		if (client_fd != sender_fd)
		// évite de renvoyer au même client si tu veux
		{
			send(client_fd, final_msg.c_str(), final_msg.size(), 0);
		}
	}
}
