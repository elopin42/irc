/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   INVITE.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <ckarsent@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:50:40 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 14:50:46 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

void Server::INVITE(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.size() < 2)
        return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " INVITE :Not enough parameters\r\n");

    std::string target_nick = cmd.args[0];
    std::string channel_name = cmd.args[1];

    if (this->channels.find(channel_name) == this->channels.end())
        return this->send_to(client->fd, ":irc.local 403 " + client->nickname + " " + channel_name + " :No such channel\r\n");

    Channel *chan = this->channels[channel_name];

    if (!chan->is_user(client->nickname))
        return this->send_to(client->fd, ":irc.local 442 " + client->nickname + " " + channel_name + " :You're not on that channel\r\n");

    if (chan->invite_only && !chan->is_operator(client->nickname))
        return this->send_to(client->fd, ":irc.local 482 " + client->nickname + " " + channel_name + " :You're not channel operator\r\n");

    Client *target = this->find_client_by_nickname(target_nick);
    if (!target)
        return this->send_to(client->fd, ":irc.local 401 " + client->nickname + " " + target_nick + " :No such nick\r\n");

    if (chan->is_user(target_nick))
        return this->send_to(client->fd, ":irc.local 443 " + client->nickname + " " + target_nick + " " + channel_name + " :is already on channel\r\n");

    chan->add_invited(target_nick);

    this->send_to(target->fd, ":" + client->nickname + "!" + client->username + "@localhost INVITE " + target_nick + " :" + channel_name + "\r\n");

    this->send_to(client->fd, ":irc.local 341 " + client->nickname + " " + target_nick + " " + channel_name + "\r\n");

    std::cout << "[INFO] " << client->nickname << " invited " << target_nick << " to " << channel_name << std::endl;
}
