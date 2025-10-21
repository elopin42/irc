/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QUIT.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <ckarsent@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:47:33 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 14:47:51 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

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
