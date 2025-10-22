/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 14:51:06 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 20:38:50 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/channel.hpp"
#include "../incl/client.hpp"
#include "../incl/server.hpp"

void Channel::bot_message(const std::string &msg) {
    std::string return_msg;
	static std::map<std::string, std::string> responses;
	
	if (responses.empty()) {
		responses["help"] = "non";
		responses["error"] = "erreur humaine detectee";
		responses["bug"] = "it works on my machine";
		responses["segfault"] = "rip";
		responses["make"] = "make re, frere";
		responses["compile"] = "ca compile pas";
		responses["malloc"] = "free oublie";
		responses["valgrind"] = "rien a signaler";
		responses["pipe"] = "ferme";
		responses["shell"] = "toujours pas fini";
        responses["norminette"] = "elle te regarde";
        responses["bash"] = "--posix";
        responses["zsh"] = "trop lent";
        responses["read"] = "rien lu";
        responses["beer"] = "pas encore";
        responses["pid"] = "zombie";
        responses["child"] = "abandonne";
        responses["thread"] = "invisible";
        responses["mutex"] = "verrouillé pour toujours";
        responses["sleep"] = "jamais";
        responses["coffee"] = "vide";
        responses["core"] = "dumped";
        responses["return"] = "rien";
        responses["void"] = "comme toi";
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