/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ckarsent <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 14:51:06 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 14:51:08 by ckarsent         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/channel.hpp"
#include "../incl/client.hpp"
#include "../incl/server.hpp"

void Channel::bot_message(const std::string &msg) {
    std::string return_msg;
	static std::map<std::string, std::string> responses;
	
	if (responses.empty()) {
		responses["1"] = "a";
		responses["2"] = "b";
		responses["3"] = "c";
		responses["4"] = "d";
		responses["5"] = "e";
		responses["6"] = "f";
		responses["7"] = "g";
		responses["8"] = "h";
		responses["9"] = "i";
		responses["10"] = "j";
	}

    std::istringstream iss(msg);
    std::string word;
    while (iss >> word) {
        std::map<std::string, std::string>::iterator it = responses.find(word);
        if (it != responses.end()) {
            return_msg = it->second;
            break;
        }
    }

    if (return_msg.empty())
        return;

    broadcast_message(":BOT!BOT@localhost PRIVMSG " + name + " :" + return_msg + "\r\n", "");
}

// void Channel::bot_message(std::string msg) {
// 	std::string return_msg;
// 	if (msg == "tu fais quoi") 
// 		return_msg = "ftg fdp";
// 	else
// 		return;
// 	broadcast_message(":BOT!BOT@localhost PRIVMSG " + name + " :" + return_msg + "\r\n", "");
// }