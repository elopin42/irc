#include "../incl/channel.hpp"
#include "../incl/server.hpp"
#include "../incl/client.hpp"

Channel::Channel(const std::string &name, Server *serv) : serv(serv), name(name), topic(""), key(""), invite_only(false), limit_user(-1), topic_restricted(false) {}

Channel::~Channel() {}

void Channel::broadcast_message(const std::string &msg, const std::string &exclude_nick)
{
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (!exclude_nick.empty() && users[i] == exclude_nick)
            continue;

        Client *u = serv->find_client_by_nickname(users[i]);
        if (u)
            u->add_to_send_buf(msg);
    }
    (void)msg;
}

void Channel::add_user(const std::string &nickname)
{
    if (std::find(users.begin(), users.end(), nickname) == users.end())
    {
        users.push_back(nickname);
    }
}

void Channel::remove_user(const std::string &nickname)
{
    std::vector<std::string>::iterator it = std::find(users.begin(), users.end(), nickname);
    if (it != users.end())
    {
        users.erase(it);
    }
}

bool Channel::is_user(const std::string &nickname)
{
    if (std::find(this->users.begin(), this->users.end(), nickname) != this->users.end())
    {
        return 1;
    }
    return 0;
}

void Channel::set_topic(const std::string &new_topic)
{
    topic = new_topic;
}

bool Channel::is_operator(const std::string &nickname) const
{
    return std::find(operators.begin(), operators.end(), nickname) != operators.end();
}

void Channel::add_operator(const Client &setter, const std::string &nickname)
{
    if (std::find(operators.begin(), operators.end(), nickname) == operators.end())
    {
        operators.push_back(nickname);
        std::ostringstream ss;
        ss << ":" << setter.nickname << "!" << setter.username << "@localhost MODE " << this->name << " +o " << nickname << "\r\n";
        this->broadcast_message(ss.str(), "");
    }
}

void Channel::remove_operator(const std::string &nickname)
{
    std::vector<std::string>::iterator it = std::find(operators.begin(), operators.end(), nickname);
    if (it != operators.end())
    {
        operators.erase(it);
    }
}

void Channel::add_invited(const std::string &nickname)
{
    if (std::find(invited_users.begin(), invited_users.end(), nickname) == invited_users.end())
        invited_users.push_back(nickname);
}

bool Channel::is_invited(const std::string &nickname) const
{
    return std::find(invited_users.begin(), invited_users.end(), nickname) != invited_users.end();
}

void Channel::remove_invited(const std::string &nickname)
{
    std::vector<std::string>::iterator it = std::find(invited_users.begin(), invited_users.end(), nickname);
    if (it != invited_users.end())
        invited_users.erase(it);
}

void Channel::set_topic(const std::string& new_topic)
{
    this->topic = new_topic;
}
