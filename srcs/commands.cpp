/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 19:26:56 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/15 19:28:05 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"
#include "../incl/channel.hpp"

void Server::JOIN(const ParsedCommand &cmd)
{
    // syntax verification a faire voir discord + plusieurs channels peuvent etre join avec une seule commande donc ajouter cette possibilite svp (simple boucle)
    if (this->channels.find(cmd.args[0]) == this->channels.end()) // does not exist yet
        this->create_channel(cmd.args[0]);
    else
    {
        if (this->channels[cmd.args[0]]->is_user(this->clients[cmd.fd]->nickname))
        {
            std::cout << "[WARN] client " << this->clients[cmd.fd]->nickname << " is already on channel " << cmd.args[0] << std::endl;
            std::ostringstream ss;
            ss << ":irc.local " << ERR_USERONCHANNEL << " "
               << this->clients[cmd.fd]->nickname << " "
               << cmd.args[0] << " :is already on channel\r\n";

            this->clients[cmd.fd]->add_to_send_buf(ss.str());
            return;
        }
    }
    this->channels[cmd.args[0]]->add_user(this->clients[cmd.fd]->nickname);
    std::cout << "[DEBUG] added client " << this->clients[cmd.fd]->nickname << " to channel " << cmd.args[0] << std::endl;
}

