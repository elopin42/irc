/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/22 14:51:06 by ckarsent          #+#    #+#             */
/*   Updated: 2025/10/22 21:10:29 by elopin           ###   ########.fr       */
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
		responses["mutex"] = "verrouille pour toujours";
		responses["sleep"] = "jamais";
		responses["coffee"] = "vide";
		responses["core"] = "dumped";
		responses["return"] = "rien";
		responses["void"] = "comme toi";
		responses["tu fais quoi"] = "je code xd";
		responses["tu as manger"] = "oui bg";
		responses["tu vas bien"] = "trkl";
		responses["yo"] = "yoooooo";
		responses["hola"] = "holaaaaaa";
		responses["y'a qui"] = "moi";
		responses["il y a qui"] = "moi enft";
		responses["bar"] = "j'y suis deja";
		responses["42"] = "the schooooooooolll";
		responses["bisous"] = "ma viiiiiiie";
		responses["il est ou ethan ?"] = "au bar";
		responses["il est ou yann ?"] = "au lidl avec jules";
	}

	std::string lower = msg;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

	if (lower.find("casino") != std::string::npos) {

		std::string symbols[] = {"🍒", "🍋", "🍊", "🍇", "🍉", "⭐", "💎"};
		std::string s1 = symbols[rand() % 7];
		std::string s2 = symbols[rand() % 7];
		std::string s3 = symbols[rand() % 7];

		if (s1 == s2 && s2 == s3)
			return_msg = s1 + s2 + s3 + " → JACKPOT 💰";
		else if (s1 == s2 || s2 == s3 || s1 == s3)
			return_msg = s1 + s2 + s3 + " → presque...";
		else
			return_msg = s1 + s2 + s3 + " → perdu";
	}

	else {
		size_t best_pos = std::string::npos;
		std::string best_response;
		for (std::map<std::string, std::string>::iterator it = responses.begin(); it != responses.end(); ++it) {
			const std::string &key = it->first;
			size_t pos = lower.find(key);
			if (pos != std::string::npos) {
				if (best_pos == std::string::npos || pos < best_pos) {
					best_pos = pos;
					best_response = it->second;
				}
			}
		}

		if (best_pos == std::string::npos)
			return;
		return_msg = best_response;
	}

	if (return_msg.empty())
		return;

	broadcast_message(":BOT!BOT@localhost PRIVMSG " + name + " :" + return_msg + "\r\n", "");
}
