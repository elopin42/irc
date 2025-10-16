#include "../incl/channel.hpp"

Channel::Channel(const std::string& name) : name(name), topic("") {}

Channel::~Channel() {}

void Channel::add_user(const std::string& nickname) {
    if (std::find(users.begin(), users.end(), nickname) == users.end()) {
        users.push_back(nickname);
    }
}

void Channel::remove_user(const std::string& nickname) {
    std::vector<std::string>::iterator it = std::find(users.begin(), users.end(), nickname);
    if (it != users.end()) {
        users.erase(it);
    }
    std::cout << "remove user" << std::endl;
}

bool Channel::is_user(const std::string &nickname) {
    if (std::find(this->users.begin(), this->users.end(), nickname) != this->users.end())
    {
        return 1;
    }
    return 0;
}

void Channel::set_topic(const std::string& new_topic) {
    topic = new_topic;
}

bool Channel::is_operator(const std::string& nickname) const {
    return std::find(operators.begin(), operators.end(), nickname) != operators.end();
}

void Channel::add_operator(const std::string& nickname) {
    if (std::find(operators.begin(), operators.end(), nickname) == operators.end()) {
        operators.push_back(nickname);
    }
}

void Channel::remove_operator(const std::string& nickname) {
    std::vector<std::string>::iterator it = std::find(operators.begin(), operators.end(), nickname);
    if (it != operators.end()) {
        operators.erase(it);
    }
}

