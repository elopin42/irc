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

	else if (lower.find("pierre") != std::string::npos
		|| lower.find("feuille") != std::string::npos
		|| lower.find("ciseaux") != std::string::npos) {
		std::string options[] = {"pierre", "feuille", "ciseaux"};
		std::string bot = options[rand() % 3];
		return_msg = "BOT a choisi " + bot + " !";
		if ((lower.find("pierre") != std::string::npos && bot == "ciseaux")
			|| (lower.find("feuille") != std::string::npos && bot == "pierre")
			|| (lower.find("ciseaux") != std::string::npos && bot == "feuille")) {
			return_msg += " => Tu gagnes !!";
		}
		else if ((lower.find("pierre") != std::string::npos && bot == "feuille")
			|| (lower.find("feuille") != std::string::npos && bot == "ciseaux")
			|| (lower.find("ciseaux") != std::string::npos && bot == "pierre")) {
			return_msg += " => BOT gagne :( ";
		}
		else {
			return_msg += " => Egalite :|";
		}
	}

	else if (lower.find("date") != std::string::npos || lower.find("heure") != std::string::npos) {
		time_t now = time(0);
		tm *ltm = localtime(&now);
		char buffer[80];
		strftime(buffer, sizeof(buffer), "Nous sommes le %Y-%m-%d et il est %H:%M:%S", ltm);
		return_msg = buffer;
	}

	else if (lower.find("calcule") != std::string::npos) {
		size_t pos = lower.find("calcule");
		std::string expression = lower.substr(pos + 7);
		std::istringstream iss(expression);
		double result;
		iss >> result;
		char op;
		while (iss >> op) {
			double next_number;
			iss >> next_number;
			if (op == '+')
				result += next_number;
			else if (op == '-')
				result -= next_number;
			else if (op == '*')
				result *= next_number;
			else if (op == '/')
				result /= next_number;
			else {
				return_msg = "Opérateur inconnu.";
				broadcast_message(":BOT!BOT@localhost PRIVMSG " + name + " :" + return_msg + "\r\n", "");
				return;
			}
		}
		std::ostringstream ss;
		ss << "Le résultat est : " << result;
		return_msg = ss.str();

	}

	else if (lower.find("pile") != std::string::npos || lower.find("face") != std::string::npos) {
		std::string toss = (rand() % 2) ? "pile" : "face";
		return_msg = "Le BOT lance la pièce... c’est " + toss + " ! ";
		if (lower.find(toss) != std::string::npos)
			return_msg += "Tu gagnes !";
		else
			return_msg += "Perdu";
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
