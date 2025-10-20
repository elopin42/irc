/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 19:26:56 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/20 19:27:25 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

// fully implemented
void Server::PRIVMSG(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.size() < 2)
        return this->send_to(client->fd, ":irc.local 411 " + client->nickname + " :No recipient given (PRIVMSG)\r\n");
    if (cmd.args[1].empty())
        return this->send_to(client->fd, ":irc.local 412 " + client->nickname + " :No text to send\r\n");

    std::string recipient = cmd.args[0];
    std::string message = cmd.args[1];

    if (recipient[0] == '#')
    {
        if (this->channels.find(recipient) == this->channels.end())
            return this->send_to(client->fd, ":irc.local 403 " + client->nickname + ' ' + recipient + " :No such channel\r\n");

        if (!this->channels[recipient]->is_user(client->nickname))
            return this->send_to(client->fd, ":irc.local 404 " + client->nickname + ' ' + recipient + " :Cannot send to channel\r\n");

        for (std::vector<std::string>::iterator it = this->channels[recipient]->users.begin();
             it != this->channels[recipient]->users.end(); ++it)
        {
            if (client->nickname != *it)
            {
                int target_fd = this->resolve_user_fd(*it);
                if (target_fd != -1)
                    this->send_to(target_fd, ":" + client->nickname + "!" + client->username + "@localhost PRIVMSG " + recipient + " :" + message + "\r\n");
            }
        }
    }
    else
    {
        int target_fd = this->resolve_user_fd(recipient);
        if (target_fd == -1)
            return this->send_to(client->fd, ":irc.local 401 " + client->nickname + ' ' + recipient + " :No such nick/channel\r\n");
        else
            this->send_to(target_fd, ":" + client->nickname + "!" + client->username + "@localhost PRIVMSG " + recipient + " :" + message + "\r\n");
    }
}

// not fully
void Server::JOIN(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (!client->registered)
        return;
    if (cmd.args.empty())
        return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " JOIN :Not enough parameters\r\n");

    // Consolidate multiple args into channel and key lists (handles spaces around commas)
    std::string channel_arg = cmd.args[0];
    std::string key_arg = "";
    
    // If there are extra args and they look like channel continuations, concatenate them
    for (size_t i = 1; i < cmd.args.size(); ++i)
    {
        if (cmd.args[i][0] == '#')
        {
            channel_arg += "," + cmd.args[i];
        }
        else if (!cmd.args[i].empty())
        {
            if (key_arg.empty())
                key_arg = cmd.args[i];
            else
                key_arg += "," + cmd.args[i];
        }
    }

    std::vector<std::string> channels = split(channel_arg, ',');
    std::vector<std::string> keys;

    if (!key_arg.empty())
        keys = split(key_arg, ',');

    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channel_name = channels[i];
        std::string key = (i < keys.size()) ? keys[i] : "";

        if (!is_valid_channel_name(channel_name))
        {
            this->send_to(client->fd, ":irc.local 403 " + client->nickname + ' ' + channel_name + " :No such channel\r\n");
            continue;
        }

        bool first_channel = false;
        if (this->channels.find(channel_name) == this->channels.end()) // does not exist yet
        {
            this->create_channel(channel_name);
            first_channel = true;
        }

        Channel *channel = this->channels[channel_name];

        if (!channel->key.empty() && key != channel->key)
        {
            this->send_to(client->fd, ":irc.local 475 " + client->nickname + ' ' + channel_name + " :Cannot join channel (+k)\r\n");
            continue;
        }

        if (channel->is_user(client->nickname))
        {
            std::cout << "[WARN] client " << client->nickname << " is already on channel " << channel_name << std::endl;
            this->send_to(client->fd, ":irc.local 443 " + client->nickname + ' ' + channel_name + " :is already on channel\r\n");
            continue;
        }

        if (channel->limit_user != -1 && channel->users.size() >= (size_t)channel->limit_user)
        {
            this->send_to(client->fd, ":irc.local 471 " + client->nickname + ' ' + channel->name + " :Cannot join channel (+l)\r\n");
            continue;
        }

        channel->add_user(client->nickname);

        std::string join_msg = ":" + client->nickname + "!" + client->username + "@localhost JOIN " + channel_name + "\r\n";
        this->send_to(client->fd, join_msg);
        
        // Broadcast JOIN to all other channel members
        for (size_t j = 0; j < channel->users.size(); ++j)
        {
            if (channel->users[j] != client->nickname)
            {
                int member_fd = this->resolve_user_fd(channel->users[j]);
                if (member_fd != -1)
                    this->send_to(member_fd, join_msg);
            }
        }

        if (first_channel)
            channel->add_operator(*client, client->nickname);
        std::cout << "[INFO] " << client->nickname << " joined " << channel_name << std::endl;

        std::string names_msg = ":irc.local 353 " + client->nickname + " = " + channel_name + " :";
        for (size_t j = 0; j < channel->users.size(); ++j)
        {
            if (channel->is_operator(channel->users[j]))
                names_msg += "@" + channel->users[j];
            else
                names_msg += channel->users[j];
            if (j != channel->users.size() - 1)
                names_msg += " ";
        }
        names_msg += "\r\n";
        this->send_to(client->fd, names_msg);

        this->send_to(client->fd, ":irc.local 366 " + client->nickname + ' ' + channel_name + " :End of NAMES list\r\n");
    }
}
// fully
void Server::PING(const ParsedCommand &cmd)
{
    if (cmd.args.empty())
        return this->send_to(cmd.fd, ":irc.local 409 " + this->clients[cmd.fd]->nickname + " :No origin specified\r\n");
    this->send_to(cmd.fd, ":irc.local PONG irc.local :" + cmd.args[0] + "\r\n");
}

// o et l de gerer mais a l'avenier on va refaire la fonction pour enlever ces gros message degueulasse car en gros c'est tout le tenmps le meme message que on reecris pour rien
void Server::MODE(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.size() < 2)
        return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n");

    std::string target = cmd.args[0];
    std::string mode = cmd.args[1];

    if (target[0] == '#')
    {
        if (this->channels.find(target) == this->channels.end())
            return this->send_to(client->fd, ":irc.local 403 " + client->nickname + " " + target + " :No such channel\r\n");

        Channel *chan = this->channels[target];

        if (!chan->is_operator(client->nickname))
            return this->send_to(client->fd, ":irc.local 482 " + client->nickname + " " + target + " :You're not channel operator\r\n");

        if (mode == "+o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan->add_operator(*client, nick);
        }
        else if (mode == "-o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan->remove_operator(nick);
            chan->broadcast_message(":" + client->nickname + " MODE " + target + " -o " + nick + "\r\n", "");
        }
        else if (mode == "+l")
        {
            if (cmd.args.size() < 3)
                return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n");
            else
            {
                chan->limit_user = std::atoi(cmd.args[2].c_str());
                chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " +l " + cmd.args[2] + "\r\n", "");
            }
        }
        else if (mode == "-l")
        {
            chan->limit_user = -1;
            chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " -l\r\n", "");
        }
        else if (mode == "+k")
        {
            if (cmd.args.size() < 3)
                return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n");
            else
            {
                chan->key = cmd.args[2];
                chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " +k " + cmd.args[2] + "\r\n", "");
            }
        }
        else if (mode == "-k")
        {
            chan->key = "";
            chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " -k\r\n", "");
        }
        else
            return this->send_to(client->fd, ":irc.local 472 " + client->nickname + " " + mode + " :is unknown mode char to me\r\n");
    }
    else
        return this->send_to(client->fd, ":irc.local 502 " + client->nickname + " :Cannot change mode for other users\r\n");
}

// fully
void Server::PONG(const ParsedCommand &cmd)
{
    // PONG is sent by the client in response to our PING
    // We do nothing
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

void Server::KICK(const ParsedCommand &cmd)
{
    Client *sender = this->clients[cmd.fd];
    if (!sender)
        return;

    if (cmd.args.size() < 2)
        return this->send_to(sender->fd, ":irc.local 461 " + sender->nickname + " KICK :Not enough parameters\r\n");

    std::string channel_name = cmd.args[0];
    std::string target_nick = cmd.args[1];
    std::string reason = (cmd.args.size() > 2) ? cmd.args[2] : "Kicked";

    if (this->channels.find(channel_name) == this->channels.end())
        return this->send_to(sender->fd, ":irc.local 403 " + sender->nickname + ' ' + channel_name + " :No such channel\r\n");

    Channel *chan = this->channels[channel_name];

    if (!chan->is_user(sender->nickname))
        return this->send_to(sender->fd, ":irc.local 442 " + sender->nickname + ' ' + channel_name + " :You're not on that channel\r\n");

    if (!chan->is_operator(sender->nickname))
        return this->send_to(sender->fd, ":irc.local 482 " + sender->nickname + ' ' + channel_name + " :You're not channel operator\r\n");

    if (!chan->is_user(target_nick))
        return this->send_to(sender->fd, ":irc.local 441 " + sender->nickname + ' ' + target_nick + ' ' + channel_name + " :They aren't on that channel\r\n");

    std::string kick_msg = ":" + sender->nickname + "!" + sender->username + "@localhost KICK " + channel_name + " " + target_nick + " :" + reason + "\r\n";

    for (std::vector<std::string>::iterator it = chan->users.begin();
         it != chan->users.end(); ++it)
    {
        int target_fd = this->resolve_user_fd(*it);
        if (target_fd != -1)
            this->send_to(target_fd, kick_msg);
    }

    chan->remove_user(target_nick);

    if (chan->is_operator(target_nick))
        chan->remove_operator(target_nick);

    std::cout << "[KICK] " << sender->nickname
              << " kicked " << target_nick
              << " from " << channel_name
              << " (" << reason << ")" << std::endl;
}

// fully
void Server::PASS(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (client->registered)
        return this->send_to(client->fd, ":irc.local 462 " + client->nickname + " :You may not reregister\r\n");

    if (cmd.args.empty())
        return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " PASS :Not enough parameters\r\n");

    client->temp_pass = cmd.args[0];
    client->pass_ok = true;
    std::cout << "[INFO] Client fd:" << client->fd << " set the temp pass" << std::endl;
}

// fully
void Server::NICK(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.empty())
        return this->send_to(client->fd, ":irc.local 431 * :No nickname given\r\n");

    if (!isValidNickname(cmd.args[0]))
        return this->send_to(client->fd, ":irc.local 432 * " + cmd.args[0] + " :Erroneous nickname\r\n");

    std::string nick = cmd.args[0];

    for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        if (it->second && it->second->nickname == nick)
            return this->send_to(client->fd, ":irc.local 433 * " + nick + " :Nickname is already in use\r\n");
    }

    client->nickname = nick;
    client->nick_ok = true;
    if (!client->first_try)
        this->try_register(client);
    std::cout << "[INFO] Client fd:" << client->fd << " nickname = " << client->nickname << std::endl;
}

// fully
void Server::USER(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];
    if (cmd.args.size() < 4)
        return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " USER :Not enough parameters\r\n");
    if (client->user_ok)
        return this->send_to(client->fd, ":irc.local 462 " + client->nickname + " :You may not reregister\r\n");
    if (!isValidUsername(cmd.args[0]))
        return this->send_to(client->fd, ":irc.local 468 * " + client->nickname + " :Invalid username\r\n");

    client->username = cmd.args[0];
    client->user_ok = true;
    if (!client->first_try)
        this->try_register(client);
    std::cout << "[INFO] Client fd:" << client->fd << " nickname = " << client->nickname << std::endl;
}

void Server::QUIT(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];
    if (!client)
        return;

    std::string reason = cmd.args.empty() ? "Client quit" : cmd.args[0];

    std::string quit_msg = ":" + client->nickname + "!" + client->username + "@localhost QUIT :" + reason + "\r\n";

    for (std::map<int, Client *>::iterator it = this->clients.begin();
         it != this->clients.end(); ++it)
    {
        Client *other = it->second;
        if (!other || other == client)
            continue;

        bool shared_channel = false;

        for (std::map<std::string, Channel *>::iterator ch = this->channels.begin();
             ch != this->channels.end(); ++ch)
        {
            Channel *chan = ch->second;
            if (!chan)
                continue;

            if (chan->is_user(client->nickname) && chan->is_user(other->nickname))
            {
                shared_channel = true;
                break;
            }
        }

        if (shared_channel)
            this->send_to(other->fd, quit_msg);
    }

    for (std::map<std::string, Channel *>::iterator it = this->channels.begin();
         it != this->channels.end(); ++it)
    {
        Channel *chan = it->second;
        if (chan && chan->is_user(client->nickname))
            chan->remove_user(client->nickname);
    }

    std::cout << "[QUIT] " << client->nickname
              << " (" << client->fd << ") → " << reason << std::endl;

    client->kick = true;
}

void Server::try_register(Client *c)
{
    if (c->registered)
        return;

    if (c->pass_ok && c->nick_ok && c->user_ok && this->password == c->temp_pass)
    {
        c->registered = true;

        std::string welcome = ":irc.local 001 " + c->nickname + " :Welcome to the Internet Relay Chat network " + c->nickname + "\r\n";
        welcome += ":irc.local 002 " + c->nickname + " :Your host is irc.local, running version 1.0\r\n";
        welcome += ":irc.local 003 " + c->nickname + " :This server was created just for 42\r\n";
        welcome += ":irc.local 004 " + c->nickname + " irc.local 1.0 o o\r\n";

        this->send_to(c->fd, welcome);
        std::cout << "[INFO] Client fd:" << c->fd << " registered succesfully!" << std::endl;
    }
    else
    {
        c->first_try = false;
        if (this->password != c->temp_pass)
        {
            this->send_to(c->fd, ":irc.local 464 * :Password incorrect\r\n");
            c->kick = true;
        }
        std::cout << "[ERROR] Client fd:" << c->fd << " failed to register!" << std::endl;
    }
}
