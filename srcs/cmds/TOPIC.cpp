/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOPIC.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 15:00:12 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 22:49:54 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

void Server::TOPIC(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.empty()) {
        std::ostringstream ss;
        ss << ":irc.local 461 " << client->nickname << " TOPIC :Not enough parameters\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }

    std::string channel_name = cmd.args[0];

    if (this->channels.find(channel_name) == this->channels.end()) {
        std::ostringstream ss;
        ss << ":irc.local 403 " << client->nickname << " " << channel_name << " :No such channel\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }

    Channel *chan = this->channels[channel_name];

    if (!chan->is_user(client->nickname)) {
        std::ostringstream ss;
        ss << ":irc.local 442 " << client->nickname << " " << channel_name << " :You're not on that channel\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }

    if (cmd.args.size() == 1) {
        if (chan->topic.empty()) {
            std::ostringstream ss;
            ss << ":irc.local 331 " << client->nickname << " " << channel_name << " :No topic is set\r\n";
            client->add_to_send_buf(ss.str());
        } else {
            std::ostringstream ss;
            ss << ":irc.local 332 " << client->nickname << " " << channel_name << " :" << chan->topic << "\r\n";
            client->add_to_send_buf(ss.str());
        }
        return;
    }

    if (chan->topic_restricted && !chan->is_operator(client->nickname)) {
        std::ostringstream ss;
        ss << ":irc.local 482 " << client->nickname << " " << channel_name << " :You're not channel operator\r\n";
        client->add_to_send_buf(ss.str());
        return;
    }

    std::string new_topic = cmd.args[1];
    chan->set_topic(new_topic);

    std::ostringstream broadcast;
    broadcast << ":" << client->nickname << "!" << client->username << "@localhost "
              << "TOPIC " << channel_name << " :" << new_topic << "\r\n";

    chan->broadcast_message(broadcast.str(), "");
}
