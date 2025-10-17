/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 19:26:56 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/16 22:19:07 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

// fully implemented
void Server::PRIVMSG(const ParsedCommand &cmd)
{
    // Check arguments
    if (cmd.args.size() < 2)
    {
        std::ostringstream ss;
        ss << ":irc.local " << ERR_NORECIPIENT << " " << this->clients[cmd.fd]->nickname << " :No recipient given (PRIVMSG)\r\n";
        this->clients[cmd.fd]->add_to_send_buf(ss.str());
        return;
    }
    // Check text
    if (cmd.args[1].empty())
    {
        std::ostringstream ss;
        ss << ":irc.local " << ERR_NOTEXTTOSEND << " " << this->clients[cmd.fd]->nickname << " :No text to send\r\n";
        this->clients[cmd.fd]->add_to_send_buf(ss.str());
        return;
    }

    std::string recipient = cmd.args[0];
    std::string message = cmd.args[1];

    if (recipient[0] == '#') // channel message
    {
        std::cout << "[DEBUG] PRIVMSG to channel " << recipient << std::endl;

        // Check channel exist
        if (this->channels.find(recipient) == this->channels.end())
        {
            std::ostringstream ss;
            ss << ":irc.local " << ERR_NOSUCHCHANNEL << " " << this->clients[cmd.fd]->nickname << " " << recipient << " :No such channel\r\n";
            this->clients[cmd.fd]->add_to_send_buf(ss.str());
            return;
        }

        // Check user in Channel
        if (!this->channels[recipient]->is_user(this->clients[cmd.fd]->nickname))
        {
            std::ostringstream ss;
            ss << ":irc.local " << ERR_CANNOTSENDTOCHAN << " " << this->clients[cmd.fd]->nickname << " " << recipient << " :Cannot send to channel\r\n";
            this->clients[cmd.fd]->add_to_send_buf(ss.str());
            return;
        }

        // Send message
        for (std::vector<std::string>::iterator it = this->channels[recipient]->users.begin();
             it != this->channels[recipient]->users.end(); ++it)
        {
            if (this->clients[cmd.fd]->nickname != *it)
            {
                int target_fd = this->resolve_user_fd(*it);
                if (target_fd != -1)
                {
                    std::ostringstream ss;
                    ss << ":" << this->clients[cmd.fd]->nickname << "!" << this->clients[cmd.fd]->username
                       << "@localhost PRIVMSG " << recipient << " :" << message << "\r\n";
                    this->clients[target_fd]->add_to_send_buf(ss.str());
                }
            }
        }
    }
    else // private message
    {
        std::cout << "[DEBUG] PRIVMSG to user " << recipient << std::endl;

        // Find user
        int target_fd = this->resolve_user_fd(recipient);
        if (target_fd == -1)
        {
            std::ostringstream ss;
            ss << ":irc.local " << ERR_NOSUCHNICK << " " << this->clients[cmd.fd]->nickname << " " << recipient << " :No such nick/channel\r\n";
            this->clients[cmd.fd]->add_to_send_buf(ss.str());
            return;
        }

        // Send message
        std::ostringstream ss;
        ss << ":" << this->clients[cmd.fd]->nickname << "!" << this->clients[cmd.fd]->username
           << "@localhost PRIVMSG " << recipient << " :" << message << "\r\n";
        this->clients[target_fd]->add_to_send_buf(ss.str());
    }
}

std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        if (!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}

bool is_valid_channel_name(const std::string &name)
{
    if (name.empty() || name[0] != '#')
        return false;
    if (name.size() > 50)
        return false;
    for (size_t i = 1; i < name.size(); ++i)
    {
        if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
            return false;
    }
    return true;
}


// not fully
void Server::JOIN(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (!this->clients[cmd.fd]->registered)
        return;
    // // syntax verification a faire voir discord + plusieurs channels peuvent etre join avec une seule commande donc ajouter cette possibilite svp (simple boucle)
    // // et enforce que le nom du channel commence par un # + interdire certains characteres precise sur discord (50 characters max # included)
    // if (this->channels.find(cmd.args[0]) == this->channels.end()) // does not exist yet
    //     this->create_channel(cmd.args[0]);
    if (cmd.args.empty())
    {
        std::ostringstream ss;
        ss << ":irc.local " << ERR_NEEDMOREPARAMS << " "
           << client->nickname << " JOIN :Not enough parameters\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }
    std::vector<std::string> channels = split(cmd.args[0], ',');
    std::vector<std::string> keys;
    if (cmd.args.size() > 1)
        keys = split(cmd.args[1], ',');
    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channel_name = channels[i];
        std::string key = (i < keys.size()) ? keys[i] : "";

        if (!is_valid_channel_name(channel_name))
        {
            std::ostringstream ss;
            ss << ":irc.local " << ERR_NOSUCHCHANNEL << " "
               << client->nickname << " " << channel_name << " :No such channel\r\n";
            client->add_to_send_buf(ss.str());
            continue;
        }

        if (this->channels.find(channel_name) == this->channels.end()) // does not exist yet
            this->create_channel(channel_name);
        
        Channel *channel = this->channels[channel_name];

        if (channel->is_user(client->nickname))
        {
            std::cout << "[WARN] client " << client->nickname << " is already on channel " << channel_name << std::endl;
            std::ostringstream ss;
            ss << ":irc.local " << ERR_USERONCHANNEL << " "
               << client->nickname << " "
               << channel_name << " :is already on channel\r\n";

            client->add_to_send_buf(ss.str());
            continue;
        }

        channel->add_user(client->nickname);
        std::cout << "[INFO] " << client->nickname << " joined " << channel_name << std::endl;\

        std::ostringstream join_msg;
        join_msg << ":" << client->nickname << "!" << client->username
                    << "@localhost JOIN " << channel_name << "\r\n";
        
        //channel->broadcast_message(client->nickname, join_msg.str());
        client->add_to_send_buf(join_msg.str());

        std::ostringstream name_list;
        name_list << ":irc.local 353 " << client->nickname << " = " << channel_name << " :";
        for (size_t j = 0; j < channel->users.size(); ++j)
        {
            name_list << channel->users[j];
            if (j != channel->users.size() - 1)
                name_list << " ";
        }
        name_list << "\r\n";
        client->add_to_send_buf(name_list.str());

        std::ostringstream end_names;
        end_names << ":irc.local 366 " << client->nickname << " " << channel_name << " :End of NAMES list\r\n";
        client->add_to_send_buf(end_names.str());
    }
    // else
    // {
    //     if (this->channels[cmd.args[0]]->is_user(this->clients[cmd.fd]->nickname))
    //     {
    //         std::cout << "[WARN] client " << this->clients[cmd.fd]->nickname << " is already on channel " << cmd.args[0] << std::endl;
    //         std::ostringstream ss;
    //         ss << ":irc.local " << ERR_USERONCHANNEL << " "
    //            << this->clients[cmd.fd]->nickname << " "
    //            << cmd.args[0] << " :is already on channel\r\n";

    //         this->clients[cmd.fd]->add_to_send_buf(ss.str());
    //         return;
    //     }
    // }
    // this->channels[cmd.args[0]]->add_user(this->clients[cmd.fd]->nickname);
    // std::cout << "[DEBUG] added client " << this->clients[cmd.fd]->nickname << " to channel " << cmd.args[0] << std::endl;
}
//49 char apres # et un seul channel a la fois pour l'instant
// fully
void Server::PING(const ParsedCommand &cmd)
{
    if (cmd.args.empty())
    {
        std::ostringstream ss;
        ss << ":irc.local 409 " << this->clients[cmd.fd]->nickname << " :No origin specified\r\n";
        this->clients[cmd.fd]->add_to_send_buf(ss.str());
    }
    else
    {
        std::ostringstream ss;
        ss << ":irc.local PONG irc.local :" << cmd.args[0] << "\r\n";
        this->clients[cmd.fd]->add_to_send_buf(ss.str());
    }
}

// fully
void Server::PONG(const ParsedCommand &cmd)
{
    // PONG is sent by the client in response to our PING
    // We can log it or do nothing
    // std::cout << "[DEBUG] Received PONG from client " << this->clients[cmd.fd]->nickname << std::endl;
    (void)cmd;
}

// fully
void Server::CAP(const ParsedCommand &cmd)
{
    if (cmd.args.size() > 0 && cmd.args[0] == "LS")
    {
        this->clients[cmd.fd]->add_to_send_buf(":irc.local CAP * LS :\r\n");
    }
    else if (cmd.args.size() > 0 && cmd.args[0] == "END")
    {
        this->try_register(this->clients[cmd.fd]);
    }
}

//fully
void Server::PASS(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (client->registered)
    {
        std::string err = ":irc.local 462 " + client->nickname + " :You may not reregister\r\n";
        client->add_to_send_buf(err);
        return;
    }

    if (cmd.args.empty())
    {
        std::string err = ":irc.local 461 " + client->nickname + " PASS :Not enough parameters\r\n";
        client->add_to_send_buf(err);
        return;
    }

    client->temp_pass = cmd.args[0];
    client->pass_ok = true;
    std::cout << "[INFO] Client fd:" << client->fd << " set the temp pass" << std::endl;
}

//fully
void Server::NICK(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.empty())
    {
        std::string err = ":irc.local 431 * :No nickname given\r\n";
        client->add_to_send_buf(err);
        return;
    }

    if (!isValidNickname(cmd.args[0]))
    {
        std::string err = ":irc.local 432 *" + cmd.args[0] + ":Erroneous nickname";
        client->add_to_send_buf(err);
        return;
    }

    std::string nick = cmd.args[0];

    for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        if (it->second && it->second->nickname == nick)
        {
            std::string err = ":irc.local 433 * " + nick + " :Nickname is already in use\r\n";
            client->add_to_send_buf(err);
            return;
        }
    }

    client->nickname = nick;
    client->nick_ok = true;
    if (!client->first_try)
        this->try_register(client);
    std::cout << "[INFO] Client fd:" << client->fd << " nickname = " << client->nickname << std::endl;
}

//fully
void Server::USER(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];
    if (cmd.args.size() < 4)
    {
        std::string err = ":irc.local 461 " + client->nickname + " USER :Not enough parameters\r\n";
        client->add_to_send_buf(err);
        return;
    }
    if (client->user_ok)
    {
        std::string err = ":irc.local 462 " + client->nickname + " :You may not reregister\r\n";
        client->add_to_send_buf(err);
        return;
    }
    if (!isValidUsername(cmd.args[0]))
    {
        std::string err = ":irc.local 468 * " + client->nickname + " :Invalid username\r\n";
        client->add_to_send_buf(err);
        return;
    }

    client->username = cmd.args[0];
    client->user_ok = true;
    if (!client->first_try)
        this->try_register(client);
    std::cout << "[INFO] Client fd:" << client->fd << " nickname = " << client->nickname << std::endl;
}

//not fully
void Server::QUIT(const ParsedCommand &cmd)
{
    //prevenir tout les channels duquel fesait parti le client avec cmd.args[0] si donne (regarder output format avec chatgpt)
    Client *client = this->clients[cmd.fd];
    client->kick = true;
}

void Server::try_register(Client *c)
{
    if (c->registered)
        return;

    if (c->pass_ok && c->nick_ok && c->user_ok && this->password == c->temp_pass)
    {
        c->registered = true;

        std::ostringstream ss;
        ss << ":irc.local 001 " << c->nickname
           << " :Welcome to the Internet Relay Chat network " << c->nickname << "\r\n";
        ss << ":irc.local 002 " << c->nickname
           << " :Your host is irc.local, running version 1.0\r\n";
        ss << ":irc.local 003 " << c->nickname
           << " :This server was created just for 42\r\n";
        ss << ":irc.local 004 " << c->nickname
           << " irc.local 1.0 o o\r\n";

        c->add_to_send_buf(ss.str());
        std::cout << "[INFO] Client fd:" << c->fd << " registered succesfully!" << std::endl;
    }
    else
    {
        c->first_try = false;
        if (this->password != c->temp_pass)
        {
            std::ostringstream ss;
            ss << ":irc.local 464 * :Password incorrect\r\n";
            c->add_to_send_buf(ss.str());
            c->kick = true;
        }
        std::cout << "[ERROR] Client fd:" << c->fd << " failed to register!" << std::endl;
    }
}
