#ifndef SERVER_HPP

#define SERVER_HPP

#include "defs.hpp"

class Client;

class Channel;

struct ParsedCommand;

class Server
{
public:
    int port;
    std::string password;

    int epfd;
    int server_fd;
    sockaddr_in addr;
    epoll_event ev, events[MAX_EVENTS];

    std::map<int, Client *> clients;
    std::map<std::string, Channel *> channels;
    std::vector<std::string> handled_commands;
    std::map<std::string, void (Server::*)(const ParsedCommand &)> command_map;

    void run(char **av);
    void epoll_loop();
    void initialize_handled_commands();
    void initialize_command_map();
    void execute_command(const ParsedCommand &cmd);

    void accept_new_client();
    void handle_client_input(int fd);
    void remove_client(int fd);
    void send_to(int fd, std::string to_send);

    int resolve_user_fd(const std::string &user);

    void create_channel(const std::string &name);
    void remove_channel(const std::string &channel);

    void PASS(const ParsedCommand &cmd);
    void NICK(const ParsedCommand &cmd);
    void USER(const ParsedCommand &cmd);
    void QUIT(const ParsedCommand &cmd);
    void PING(const ParsedCommand &cmd);
    void PONG(const ParsedCommand &cmd);
    void CAP(const ParsedCommand &cmd);
    void try_register(Client *c);

    void PRIVMSG(const ParsedCommand &cmd);
    void NOTICE(const ParsedCommand &cmd);

    void JOIN(const ParsedCommand &cmd);
    void PART(const ParsedCommand &cmd);
    void TOPIC(const ParsedCommand &cmd);
    void MODE(const ParsedCommand &cmd);
    void INVITE(const ParsedCommand &cmd);
    Client *find_client_by_nickname(const std::string &nickname);

    void KICK(const ParsedCommand &cmd);

    void WHO(const ParsedCommand &cmd);
    void WHOIS(const ParsedCommand &cmd);
    void NAMES(const ParsedCommand &cmd);
    void LIST(const ParsedCommand &cmd);
    void ERROR(const ParsedCommand &cmd);
};

#endif
