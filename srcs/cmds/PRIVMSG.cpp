/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PRIVMSG.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <ckarsent@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:52:19 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 14:52:25 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

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
