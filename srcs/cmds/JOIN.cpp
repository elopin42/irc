/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   JOIN.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:52:00 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 22:53:39 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

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
        if (!cmd.args[i].empty() && cmd.args[i][0] == '#')
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

        if (channel->invite_only && !channel->is_invited(client->nickname))
        {
            this->send_to(client->fd, ":irc.local 473 " + client->nickname + " " + channel_name + " :Cannot join channel (+i)\r\n");
            continue;
        }
        if (channel->invite_only)
          channel->remove_invited(client->nickname);

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

        // Track joined channel in client
        if (std::find(client->joined_channels.begin(), client->joined_channels.end(), channel_name) == client->joined_channels.end())
            client->joined_channels.push_back(channel_name);

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

        if (channel->topic.empty()) {
            std::ostringstream ss;
            ss << ":irc.local 331 " << client->nickname << " " << channel_name << " :No topic is set\r\n";
            client->add_to_send_buf(ss.str());
        } else {
            std::ostringstream ss;
            ss << ":irc.local 332 " << client->nickname << " " << channel_name << " :" << channel->topic << "\r\n";
            client->add_to_send_buf(ss.str());
        }

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
