/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   USER.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <ckarsent@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:48:25 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 14:48:35 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

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
