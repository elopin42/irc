/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   KICK.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:49:35 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 22:53:39 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

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

    // Remove from kicked user's joined_channels tracking
    Client *kicked_client = this->find_client_by_nickname(target_nick);
    if (kicked_client)
    {
        std::vector<std::string>::iterator it = std::find(kicked_client->joined_channels.begin(), kicked_client->joined_channels.end(), channel_name);
        if (it != kicked_client->joined_channels.end())
            kicked_client->joined_channels.erase(it);
    }

    if (chan->is_operator(target_nick))
        chan->remove_operator(target_nick);

    std::cout << "[KICK] " << sender->nickname
              << " kicked " << target_nick
              << " from " << channel_name
              << " (" << reason << ")" << std::endl;
}
