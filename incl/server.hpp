/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/01 14:33:51 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/16 16:10:25 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

    int resolve_user_fd(const std::string &user);

    void create_channel(const std::string &name);
    void remove_channel(const std::string &channel);//not implemented/rework
    // bool look_channel(const std::string &channel, int fd);   not really correctly implemented/ did not look enought into it please make sure it's useful
    // void broadcast_message(int sender_fd, const std::string &msg);
    // bool share_channel(int fd1, int fd2);

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
    //Pretty much every command here, has error codes you have to send, not only in case of errors but also in case of success,//
    //Please ask chatgpt for how to format your message for every different situation for irssi to handle it correctly         //
    //AND NEVER USE SEND DIRECTLY, ONLY USE ADD_TO_SEND_BUFFER AS THE CLIENT                                                   //
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
    
    // Connection / registration
    void PASS(const ParsedCommand &cmd); //sets the pass of the client
    void NICK(const ParsedCommand &cmd); //sets the nick of the client
    void USER(const ParsedCommand &cmd); //sets the username of the client
    void QUIT(const ParsedCommand &cmd); //Disconnects the client cleanly from the server CTRL C sends quit on irc, but nc just sends 0 bytes so call this function when n = recv && n = 0
    void PING(const ParsedCommand &cmd); //sends PONG to the client example Client: "PING :1724016534"  Server: "PONG :1724016534"
    void PONG(const ParsedCommand &cmd); //we can send a PING ourselves to the client so we have the handle receiving PONG also but basically does nothing
    void CAP(const ParsedCommand &cmd);  //handle CAP LS / CAP END
    void try_register(Client *c);

    // Messaging
    void PRIVMSG(const ParsedCommand &cmd); //sends a message to a channel or a client, you know it's a channel when it's preceded by #
    void NOTICE(const ParsedCommand &cmd); //Same as private message but if there was an error with the message (eg no params, no args we don't send an error we do nothing)

    // Channels
    void JOIN(const ParsedCommand &cmd); //joins a channel, has to be preceded by # and 49 characters long maximum 1 character minimum
    void PART(const ParsedCommand &cmd); //leave a channel, if the channel is empty, delete it, for bonus don't forget to delete if the bot is alone in it
    void TOPIC(const ParsedCommand &cmd);//2 usages: TOPIC <channel>, you have to just send the topic to the user, TOPIC <channel> :text here you have to change the topic of the channel and broadcast the new one to users
    void MODE(const ParsedCommand &cmd); //changes some things about the channel you are in, everything is described in the subject some or all may require being operator of the channel
    void INVITE(const ParsedCommand &cmd);//invites someone to the channel, sends a notification to the invited client, the client can then use join, (add a vector in the channel class that will work as an
    //invite list, when a user invites someone that person is added to the list, and when the user in question joins, you remove his name from the invite list, beware of the fact that the channel may not
    //be always invite only, so even check the invite list only if needed if not needed still remove the name from the invite list as the channel may go invite only later)
    
    void KICK(const ParsedCommand &cmd);//kicks a user from the channel, not the server

    void WHO(const ParsedCommand &cmd);   // Optional: list users in a channel
    void WHOIS(const ParsedCommand &cmd); // Optional: info on a user
    void NAMES(const ParsedCommand &cmd); // Optional: show members of a channel
    void LIST(const ParsedCommand &cmd);  // Optional: list channels
    void ERROR(const ParsedCommand &cmd); // Optional: internal errors ?

};

int check_join_command(const std::string &msg, std::string &out_channel);// ????????????

#endif
