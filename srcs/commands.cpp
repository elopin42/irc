/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 19:26:56 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/19 01:09:46 by elopin           ###   ########.fr       */
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

// not fully
// :irc.example.com 461 <nick> JOIN :Invalid syntax – do not use spaces in channel list
//dans le cas d'espace au mauvais endroit
void Server::JOIN(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (!this->clients[cmd.fd]->registered)
        return;
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
}
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

void Server::MODE(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.size() < 2)
    {
        std::string err = ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n";
        client->add_to_send_buf(err);
        return;
    }

    std::string target = cmd.args[0];
    std::string mode = cmd.args[1];

    if (target[0] == '#')
    {
        if (this->channels.find(target) == this->channels.end())
        {
            std::string err = ":irc.local 403 " + client->nickname + " " + target + " :No such channel\r\n";
            client->add_to_send_buf(err);
            return;
        }

        Channel &chan = this->channels[target];

        if (!chan.is_operator(client->nickname))
        {
            std::string err = ":irc.local 482 " + client->nickname + " " + target + " :You're not channel operator\r\n";
            client->add_to_send_buf(err);
            return;
        }

        if (mode == "+o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan.add_operator(nick);
            std::string msg = ":" + client->nickname + " MODE " + target + " +o " + nick + "\r\n";
            for (size_t i = 0; i < chan.users.size(); ++i)
            {
                Client *u = find_client_by_nickname(chan.users[i]);
                if (u)
                    u->add_to_send_buf(msg);
            }
        }
        else if (mode == "-o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan.remove_operator(nick);
            std::string msg = ":" + client->nickname + " MODE " + target + " -o " + nick + "\r\n";
            for (size_t i = 0; i < chan.users.size(); ++i)
            {
                Client *u = find_client_by_nickname(chan.users[i]);
                if (u)
                    u->add_to_send_buf(msg);
            }
        }
        else
        {
            std::string err = ":irc.local 472 " + client->nickname + " " + mode + " :is unknown mode char to me\r\n";
            client->add_to_send_buf(err);
        }
    }
    else
    {
        std::string err = ":irc.local 502 " + client->nickname + " :Cannot change mode for other users\r\n";
        client->add_to_send_buf(err);
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

void Server::KICK(const ParsedCommand &cmd)
{
	Client *sender = this->clients[cmd.fd];
	if (!sender)
		return ;

    if (cmd.args.size() < 2)
    {
        std::ostringstream err;
        err << ":irc.local 461 " << sender->nickname << " KICK :Not enough parameters\r\n";
        sender->add_to_send_buf(err.str());
        return;
    }

    std::string channel_name = cmd.args[0];
    std::string target_nick = cmd.args[1];
    std::string reason = (cmd.args.size() > 2) ? cmd.args[2] : "Kicked";

    if (this->channels.find(channel_name) == this->channels.end())
    {
        std::ostringstream err;
        err << ":irc.local 403 " << sender->nickname << " " << channel_name << " :No such channel\r\n";
        sender->add_to_send_buf(err.str());
        return;
    }
	
	Channel *chan = this->channels[channel_name];

    if (!chan->is_user(sender->nickname))
    {
        std::ostringstream err;
        err << ":irc.local 442 " << sender->nickname << " " << channel_name << " :You're not on that channel\r\n";
        sender->add_to_send_buf(err.str());
        return;
    }

    if (!chan->is_operator(sender->nickname))
    {
        std::ostringstream err;
        err << ":irc.local 482 " << sender->nickname << " " << channel_name << " :You're not channel operator\r\n";
        sender->add_to_send_buf(err.str());
        return;
    }

    if (!chan->is_user(target_nick))
    {
        std::ostringstream err;
        err << ":irc.local 441 " << sender->nickname << " " << target_nick << " " << channel_name << " :They aren't on that channel\r\n";
        sender->add_to_send_buf(err.str());
        return;
    }

    std::ostringstream ss;
    ss << ":" << sender->nickname << "!" << sender->username
       << "@localhost KICK " << channel_name << " "
       << target_nick << " :" << reason << "\r\n";
    std::string kick_msg = ss.str();

    for (std::vector<std::string>::iterator it = chan->users.begin();
         it != chan->users.end(); ++it)
    {
        int target_fd = this->resolve_user_fd(*it);
        if (target_fd != -1)
            this->clients[target_fd]->add_to_send_buf(kick_msg);
    }

    chan->remove_user(target_nick);

    if (chan->is_operator(target_nick))
        chan->remove_operator(target_nick);

    std::cout << "[KICK] " << sender->nickname
              << " kicked " << target_nick
              << " from " << channel_name
              << " (" << reason << ")" << std::endl;


}

void Server::MODE(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    // Vérifie arguments
    if (cmd.args.size() < 2)
    {
        std::string err = ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n";
        client->add_to_send_buf(err);
        return;
    }

    std::string target = cmd.args[0];
    std::string mode = cmd.args[1];

    // --- MODE sur un channel ---
    if (target[0] == '#')
    {
        if (this->channels.find(target) == this->channels.end())
        {
            std::string err = ":irc.local 403 " + client->nickname + " " + target + " :No such channel\r\n";
            client->add_to_send_buf(err);
            return;
        }

        Channel &chan = this->channels[target];

        // Seuls les opérateurs peuvent changer les modes
        if (!chan.is_operator(client->nickname))
        {
            std::string err = ":irc.local 482 " + client->nickname + " " + target + " :You're not channel operator\r\n";
            client->add_to_send_buf(err);
            return;
        }

        // Gérer le mode
        if (mode == "+o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan.add_operator(nick);
            std::string msg = ":" + client->nickname + " MODE " + target + " +o " + nick + "\r\n";
            // broadcast à tous les users du channel
            for (size_t i = 0; i < chan.users.size(); ++i)
            {
                Client *u = find_client_by_nickname(chan.users[i]);
                if (u)
                    u->add_to_send_buf(msg);
            }
        }
        else if (mode == "-o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan.remove_operator(nick);
            std::string msg = ":" + client->nickname + " MODE " + target + " -o " + nick + "\r\n";
            for (size_t i = 0; i < chan.users.size(); ++i)
            {
                Client *u = find_client_by_nickname(chan.users[i]);
                if (u)
                    u->add_to_send_buf(msg);
            }
        }
        else
        {
            std::string err = ":irc.local 472 " + client->nickname + " " + mode + " :is unknown mode char to me\r\n";
            client->add_to_send_buf(err);
        }
    }
    else
    {
        // MODE sur un user (souvent non implémenté dans ft_irc)
        std::string err = ":irc.local 502 " + client->nickname + " :Cannot change mode for other users\r\n";
        client->add_to_send_buf(err);
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

void Server::QUIT(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];
    if (!client)
        return;

    std::string reason = cmd.args.empty() ? "Client quit" : cmd.args[0];

    std::ostringstream ss;
    ss << ":" << client->nickname << "!" << client->username
       << "@localhost QUIT :" << reason << "\r\n";
    std::string quit_msg = ss.str();

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
            other->add_to_send_buf(quit_msg);
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
