/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PART.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:59:33 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 18:29:24 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

void Server::PART(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (!client->registered)
        return;
    
    if (cmd.args.empty())
        return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " PART :Not enough parameters\r\n");

    std::string channel_arg = cmd.args[0];
    std::string part_message = "";

    for (size_t i = 1; i < cmd.args.size(); ++i)
    {
        if (!cmd.args[i].empty() && cmd.args[i][0] == '#')
            channel_arg += "," + cmd.args[i];
        else if (!part_message.empty())
            part_message += " " + cmd.args[i];
        else
            part_message = cmd.args[i];
    }

    std::vector<std::string> channels = split(channel_arg, ',');

    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channel_name = channels[i];

        if (this->channels.find(channel_name) == this->channels.end())
        {
            this->send_to(client->fd, ":irc.local 403 " + client->nickname + ' ' + channel_name + " :No such channel\r\n");
            continue;
        }

        Channel *channel = this->channels[channel_name];

        if (!channel->is_user(client->nickname))
        {
            this->send_to(client->fd, ":irc.local 442 " + client->nickname + ' ' + channel_name + " :You're not on that channel\r\n");
            continue;
        }

        std::string part_msg = ":" + client->nickname + "!" + client->username + "@localhost PART " + channel_name;
        if (!part_message.empty())
            part_msg += " :" + part_message;
        part_msg += "\r\n";

        // Broadcast
        for (size_t j = 0; j < channel->users.size(); ++j)
        {
            int user_fd = this->resolve_user_fd(channel->users[j]);
            if (user_fd != -1)
                this->send_to(user_fd, part_msg);
        }

        channel->remove_user(client->nickname);

        if (channel->is_operator(client->nickname))
            channel->remove_operator(client->nickname);

        std::cout << "[INFO] " << client->nickname << " parted " << channel_name;
        if (!part_message.empty())
            std::cout << " (" << part_message << ")";
        std::cout << std::endl;

        if (channel->users.empty())
            this->remove_channel(channel->name);
    }
}

