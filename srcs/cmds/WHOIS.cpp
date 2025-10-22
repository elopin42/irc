/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WHOIS.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:55:04 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 22:50:04 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

void Server::WHOIS(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.empty()) {
        std::ostringstream ss;
        ss << ":irc.local 431 " << client->nickname << " :No nickname given\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }

    std::string target_nick = cmd.args[0];
    Client *target = this->find_client_by_nickname(target_nick);

    if (!target) {
        std::ostringstream ss;
        ss << ":irc.local 401 " << client->nickname << " " << target_nick << " :No such nick\r\n";
        ss << ":irc.local 318 " << client->nickname << " " << target_nick << " :End of /WHOIS list\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }

    std::ostringstream info;
    info << ":irc.local 311 " << client->nickname << " " << target->nickname
         << " " << target->username << " localhost * :" << target->realname << "\r\n";
    client->add_to_send_buf(info.str());

    std::ostringstream channels_list;
    channels_list << ":irc.local 319 " << client->nickname << " " << target->nickname << " :";

    bool first = true;
    for (std::map<std::string, Channel*>::iterator it = this->channels.begin(); it != this->channels.end(); ++it) {
        Channel *chan = it->second;
        if (chan->is_user(target->nickname)) {
            if (!first) channels_list << " ";
            if (chan->is_operator(target->nickname))
                channels_list << "@";
            channels_list << chan->name;
            first = false;
        }
    }
    channels_list << "\r\n";
    client->add_to_send_buf(channels_list.str());

    std::ostringstream serverinfo;
    serverinfo << ":irc.local 312 " << client->nickname << " " << target->nickname
               << " irc.local :42 IRC Server\r\n";
    client->add_to_send_buf(serverinfo.str());

    std::ostringstream end;
    end << ":irc.local 318 " << client->nickname << " " << target->nickname << " :End of /WHOIS list\r\n";
    client->add_to_send_buf(end.str());
}
