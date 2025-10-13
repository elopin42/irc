#include "../incl/struct_class.hpp"
#include "../incl/server.hpp"
#include "../incl/defs.hpp"

int find_channel_by_fd(const std::map<int, std::vector<Client> > &channels, int fd_to_find)
{
    for (std::map<int, std::vector<Client> >::const_iterator it = channels.begin();
         it != channels.end(); ++it)
    {
        int channel_id = it->first;
        const std::vector<Client> &clients = it->second;

        for (size_t i = 0; i < clients.size(); ++i)
        {
            if (clients[i].fd == fd_to_find)
                return channel_id;
        }
    }
    return -1;
}

Client find_client_by_fd(const std::map<int, std::vector<Client> > &channels, int fd_to_find)
{
    for (std::map<int, std::vector<Client> >::const_iterator it = channels.begin();
         it != channels.end(); ++it)
    {
        const std::vector<Client> &clients = it->second;

        for (size_t i = 0; i < clients.size(); ++i)
        {
            if (clients[i].fd == fd_to_find)
                return clients[i];
        }
    }

    Client empty;
    empty.fd = -1;
    return empty;
}


void broadcast_message(std::vector<Client> *clients, int sender_fd, const std::string &msg)
{
    std::stringstream ss;
    ss << "[Client " << sender_fd << "] " << msg;
    std::string final_msg = ss.str();

    for (size_t i = 0; i < clients->size(); i++)
    {
        int client_fd = (*clients)[i].fd;
        if (client_fd != sender_fd) // évite de renvoyer au même client si tu veux
        {
            send(client_fd, final_msg.c_str(), final_msg.size(), 0);
        }
    }
}
