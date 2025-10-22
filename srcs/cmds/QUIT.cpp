/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QUIT.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:47:33 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 22:53:39 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

void Server::QUIT(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];
    if (!client)
        return;

    std::string reason = cmd.args.empty() ? "Client quit" : cmd.args[0];

    std::string quit_msg = ":" + client->nickname + "!" + client->username + "@localhost QUIT :" + reason + "\r\n";

    // Notify all clients in shared channels (more efficient using joined_channels)
    std::set<std::string> notified_channels;
    for (size_t i = 0; i < client->joined_channels.size(); ++i)
    {
        Channel *chan = this->channels[client->joined_channels[i]];
        if (chan)
        {
            for (size_t j = 0; j < chan->users.size(); ++j)
            {
                if (chan->users[j] != client->nickname)
                {
                    int user_fd = this->resolve_user_fd(chan->users[j]);
                    if (user_fd != -1)
                        this->send_to(user_fd, quit_msg);
                }
            }
        }
    }

    // Remove client from all channels
    for (size_t i = 0; i < client->joined_channels.size(); ++i)
    {
        Channel *chan = this->channels[client->joined_channels[i]];
        if (chan)
        {
            chan->remove_user(client->nickname);
            if (chan->is_operator(client->nickname))
                chan->remove_operator(client->nickname);
            if (chan->users.empty())
                this->remove_channel(chan->name);
        }
    }

    std::cout << "[QUIT] " << client->nickname
              << " (" << client->fd << ") → " << reason << std::endl;

    client->kick = true;
}
