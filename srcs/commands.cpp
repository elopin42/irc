/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <ckarsent@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 19:26:56 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/21 14:52:16 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

void Server::try_register(Client *c)
{
    if (c->registered)
        return;

    if (c->pass_ok && c->nick_ok && c->user_ok && this->password == c->temp_pass)
    {
        c->registered = true;

        std::string welcome = ":irc.local 001 " + c->nickname + " :Welcome to the Internet Relay Chat network " + c->nickname + "\r\n";
        welcome += ":irc.local 002 " + c->nickname + " :Your host is irc.local, running version 1.0\r\n";
        welcome += ":irc.local 003 " + c->nickname + " :This server was created just for 42\r\n";
        welcome += ":irc.local 004 " + c->nickname + " irc.local 1.0 o o\r\n";

        this->send_to(c->fd, welcome);
        std::cout << "[INFO] Client fd:" << c->fd << " registered succesfully!" << std::endl;
    }
    else
    {
        c->first_try = false;
        if (this->password != c->temp_pass)
        {
            this->send_to(c->fd, ":irc.local 464 * :Password incorrect\r\n");
            c->kick = true;
        }
        std::cout << "[ERROR] Client fd:" << c->fd << " failed to register!" << std::endl;
    }
}
