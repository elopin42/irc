#include "../incl/channel.hpp"

Channel::Channel(const std::string& name) : name(name), topic("") {}

Channel::~Channel() {}

void Channel::add_user(const std::string& nickname) {
    if (users.find(nickname) == users.end()) {
        users.insert(nickname);
    }
}

void Channel::remove_user(const std::string& nickname) {
    users.erase(nickname);
}

void Channel::set_topic(const std::string& new_topic) {
    topic = new_topic;
}

bool Channel::is_operator(const std::string& nickname) const {
    return operators.find(nickname) != operators.end();
}

void Channel::add_operator(const std::string& nickname) {
    operators.insert(nickname);
}

void Channel::remove_operator(const std::string& nickname) {
    operators.erase(nickname);
}

#include "../incl/server.hpp"
#include "../incl/client.hpp"

void Server::join_channel(const std::string &channel, int fd) {
    std::vector<std::string> &joined = this->clients[fd]->joined_channels;

    if (std::find(joined.begin(), joined.end(), channel) == joined.end()) {
        joined.push_back(channel);
        std::cout << "[INFO] Client " << fd << " joined channel " << channel << std::endl;
        this->clients[fd]->channel_ok = true;
    } else {
        std::cout << "[WARN] Client " << fd << " already in channel " << channel << std::endl;
    }
}

void Server::remove_channel(const std::string &channel, int fd) {
    std::vector<std::string> &joined = this->clients[fd]->joined_channels;

    std::vector<std::string>::iterator it = std::find(joined.begin(), joined.end(), channel);
    if (it != joined.end()) {
        joined.erase(it);
        std::cout << "[INFO] Client " << fd << " removed channel " << channel << std::endl;
        this->clients[fd]->channel_ok = false;
    } else {
        std::cout << "[WARN] Client " << fd << " no such channel " << channel << std::endl;
    }
}

bool Server::look_channel(const std::string &channel, int fd) {
    std::vector<std::string> &joined = this->clients[fd]->joined_channels;

    if (std::find(joined.begin(), joined.end(), channel) != joined.end()) {
      return true;
    } else {
      return false;
    }
}

int check_join_command(const std::string &msg, std::string &out_channel) {
    std::istringstream iss(msg);
    std::string command;
    iss >> command >> out_channel;

    if (command == "JOIN" && !out_channel.empty())
        return 1; // valide
    return -1; // pas un JOIN
}

bool Server::share_channel(int fd1, int fd2)
{
    const std::vector<std::string>& ch1 = this->clients[fd1]->joined_channels;
    const std::vector<std::string>& ch2 = this->clients[fd2]->joined_channels;

    for (size_t i = 0; i < ch1.size(); ++i)
    {
        for (size_t j = 0; j < ch2.size(); ++j)
        {
            if (ch1[i] == ch2[j])
                return true; // ✅ au moins un channel en commun
        }
    }
    return false; // ❌ aucun channel partagé
}

void Server::broadcast_message(int sender_fd, const std::string &msg)
{
    std::stringstream ss;
    ss << "[client " << sender_fd << "] " << msg;
    std::string final_msg = ss.str();
    
    // 🔹 Cas 1 : le client n'a pas de channel
    if (!this->clients[sender_fd]->channel_ok)
    {
        for (std::map<int, Client*>::iterator it = this->clients.begin();
             it != this->clients.end(); ++it)
        {
            Client* client = it->second;
            if (!client)
                continue;
            int client_fd = client->fd;
            
            // ❌ évite d'envoyer à soi-même
            if (client_fd == sender_fd)
                continue;
            
            // saute ceux qui sont dans un channel
            if (this->clients[client_fd]->channel_ok)
                continue;
            
            send(client_fd, final_msg.c_str(), final_msg.size(), 0);
        }
        std::cout << "[broadcast] from client " << sender_fd << ": " << msg << std::endl;
    }
    // 🔹 Cas 2 : le client appartient à un channel
    else
    {
        for (std::map<int, Client*>::iterator it = this->clients.begin();
             it != this->clients.end(); ++it)
        {
            Client* client = it->second;
            if (!client)
                continue;
            int client_fd = client->fd;
            
            // ❌ évite d'envoyer à soi-même
            if (client_fd == sender_fd)
                continue;
            
            // on saute ceux qui n'ont PAS de channel
            if (!this->clients[client_fd]->channel_ok)
                continue;
            
            // on vérifie qu'ils partagent un channel avec le sender
            if (!this->share_channel(sender_fd, client_fd))
                continue;
            
            send(client_fd, final_msg.c_str(), final_msg.size(), 0);
        }
        std::cout << "[broadcast] from client " << sender_fd << ": " << msg << std::endl;
    }
}
