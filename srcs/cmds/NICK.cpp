/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NICK.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:48:47 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 22:53:39 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

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

    std::string old_nick = client->nickname;
    client->nickname = nick;
    client->nick_ok = true;

    // Broadcast NICK change to all shared channels and update their user lists
    if (client->registered)
    {
        std::string nick_msg = ":" + old_nick + "!" + client->username + "@localhost NICK :" + nick + "\r\n";
        for (std::map<std::string, Channel *>::iterator ch = this->channels.begin(); 
             ch != this->channels.end(); ++ch)
        {
            if (ch->second && ch->second->is_user(old_nick))
            {
                // Update channel's user list with new nickname
                std::vector<std::string>::iterator user_it = std::find(ch->second->users.begin(), 
                                                                       ch->second->users.end(), old_nick);
                if (user_it != ch->second->users.end())
                    *user_it = nick;
                
                ch->second->broadcast_message(nick_msg, "");
            }
        }
    }

    if (!client->first_try)
        this->try_register(client);
    std::cout << "[INFO] Client fd:" << client->fd << " nickname changed to " << client->nickname << std::endl;
}
