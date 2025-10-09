#include "../incl/server.hpp"
#include "../incl/struct_class.hpp"

int add_client(int epfd, int client_fd, std::vector<Client> *clients, epoll_event ev)
{
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
    {
        perror("epoll_ctl ADD client");
        return -1;
    }

    clients->push_back(Client(client_fd, "unknown", 0, 1));

    std::cout << "[INFO] New client added (fd=" << client_fd << ")" << std::endl;
    return 0;
}

int remove_client(int epfd, int client_fd, std::vector<Client> *clients)
{
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL) == -1)
    {
        perror("epoll_ctl DEL client");
        return -1;
    }

    close(client_fd);

    for (size_t i = 0; i < clients->size(); i++)
    {
        if ((*clients)[i].fd == client_fd)
        {
            clients->erase(clients->begin() + i);
            break;
        }
    }

    std::cout << "[INFO] Client removed (fd=" << client_fd << ")" << std::endl;
    return 0;
}

void Client::appendToBuffer(const std::string &message)
{
    _output_buffer += message;
}
 
void handle_client_input(Client &client, const std::string &data) {
    client._input_buffer += data;

    size_t pos;
    while ((pos = client._input_buffer.find('\n')) != std::string::npos) {
        std::string line = client._input_buffer.substr(0, pos + 1);
        client._input_buffer.erase(0, pos + 1);

        if (line.size() >= 2 && line[line.size() - 2] == '\r' && line[line.size() - 1] == '\n') {
        } else {
            if (!line.empty() && line[line.size() - 1] == '\n') {
                line.erase(line.size() - 1);
                line += "\r\n";
            }
        }

        if (line.size() > 512) {
            line = line.substr(0, 510) + "\r\n";
        }

        std::cout << "[RECV] from fd=" << client.fd << " -> " << line;

        client.appendToBuffer(line);
    }
}

void broadcast_message(std::vector<Client> *clients, int sender_fd, const std::string &msg)
{
    for (size_t i = 0; i < clients->size(); i++)
    {
        int client_fd = (*clients)[i].fd;
        if (client_fd != sender_fd) // évite de renvoyer au même client si tu veux
        {
            send(client_fd, msg.c_str(), msg.size(), 0);
        }
    }
}

