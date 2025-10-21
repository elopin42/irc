/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PING.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <ckarsent@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:51:36 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 14:51:41 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

void Server::PING(const ParsedCommand &cmd)
{
    if (cmd.args.empty())
        return this->send_to(cmd.fd, ":irc.local 409 " + this->clients[cmd.fd]->nickname + " :No origin specified\r\n");
    this->send_to(cmd.fd, ":irc.local PONG irc.local :" + cmd.args[0] + "\r\n");
}
