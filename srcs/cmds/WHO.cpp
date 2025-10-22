/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WHO.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:54:56 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 22:51:29 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

void Server::WHO(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.empty()) {
        std::ostringstream ss;
        ss << ":irc.local 461 " << client->nickname << " WHO :Not enough parameters\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }

    std::string target = cmd.args[0];

    if (target[0] == '#') {
        if (this->channels.find(target) == this->channels.end()) {
            std::ostringstream ss;
            ss << ":irc.local 403 " << client->nickname << " " << target << " :No such channel\r\n";
            client->add_to_send_buf(ss.str());
            return;
        }

        Channel *chan = this->channels[target];
        for (size_t i = 0; i < chan->users.size(); ++i) {
            Client *u = this->find_client_by_nickname(chan->users[i]);
            if (!u) continue;

            std::ostringstream ss;
            ss << ":irc.local 352 " << client->nickname << " " << target
               << " " << u->username
               << " localhost irc.local "
               << u->nickname << " H";

            if (chan->is_operator(u->nickname))
                ss << "@";

            ss << " :0 " << u->realname << "\r\n";
            client->add_to_send_buf(ss.str());
        }

        std::ostringstream end;
        end << ":irc.local 315 " << client->nickname << " " << target << " :End of /WHO list\r\n";
        client->add_to_send_buf(end.str());
    }

    else {
        Client *u = this->find_client_by_nickname(target);
        if (u) {
            std::ostringstream ss;
            ss << ":irc.local 352 " << client->nickname << " * "
               << u->username << " localhost irc.local "
               << u->nickname << " H :0 " << u->realname << "\r\n";
            client->add_to_send_buf(ss.str());
        }

        std::ostringstream end;
        end << ":irc.local 315 " << client->nickname << " " << target << " :End of /WHO list\r\n";
        client->add_to_send_buf(end.str());
    }
}
