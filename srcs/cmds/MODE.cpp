/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MODE.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <ckarsent@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/21 14:51:17 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/21 14:51:22 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include "../incl/client.hpp"

// o et l de gerer mais a l'avenier on va refaire la fonction pour enlever ces gros message degueulasse car en gros c'est tout le tenmps le meme message que on reecris pour rien
void Server::MODE(const ParsedCommand &cmd)
{
    Client *client = this->clients[cmd.fd];

    if (cmd.args.size() < 2)
        return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n");

    std::string target = cmd.args[0];
    std::string mode = cmd.args[1];

    if (target[0] == '#')
    {
        if (this->channels.find(target) == this->channels.end())
            return this->send_to(client->fd, ":irc.local 403 " + client->nickname + " " + target + " :No such channel\r\n");

        Channel *chan = this->channels[target];

        if (!chan->is_operator(client->nickname))
            return this->send_to(client->fd, ":irc.local 482 " + client->nickname + " " + target + " :You're not channel operator\r\n");

        if (mode == "+o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan->add_operator(*client, nick);
        }
        else if (mode == "-o" && cmd.args.size() >= 3)
        {
            std::string nick = cmd.args[2];
            chan->remove_operator(nick);
            chan->broadcast_message(":" + client->nickname + " MODE " + target + " -o " + nick + "\r\n", "");
        }
        else if (mode == "+l")
        {
            if (cmd.args.size() < 3)
                return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n");
            else
            {
                chan->limit_user = std::atoi(cmd.args[2].c_str());
                chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " +l " + cmd.args[2] + "\r\n", "");
            }
        }
        else if (mode == "-l")
        {
            chan->limit_user = -1;
            chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " -l\r\n", "");
        }
        else if (mode == "+k")
        {
            if (cmd.args.size() < 3)
                return this->send_to(client->fd, ":irc.local 461 " + client->nickname + " MODE :Not enough parameters\r\n");
            else
            {
                chan->key = cmd.args[2];
                chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " +k " + cmd.args[2] + "\r\n", "");
            }
        }
        else if (mode == "-k")
        {
            chan->key = "";
            chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " -k\r\n", "");
        }
        else if (mode == "+i")
        {
          chan->invite_only = true;
          chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " +t\r\n", "");
        }
        else if (mode == "-i")
        {
          chan->invite_only = false;
          chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " +i\r\n", "");
        }
        else if (mode == "+t")
        {
            chan->topic_restricted = true;
            chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " +t\r\n", "");
        }
        else if (mode == "-t")
        {
            chan->topic_restricted = false;
            chan->broadcast_message(":" + client->nickname + " MODE " + chan->name + " -t\r\n", "");
        }
        else
            return this->send_to(client->fd, ":irc.local 472 " + client->nickname + " " + mode + " :is unknown mode char to me\r\n");
    }
    else
        return this->send_to(client->fd, ":irc.local 502 " + client->nickname + " :Cannot change mode for other users\r\n");
}
