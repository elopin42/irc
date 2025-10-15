/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 17:54:15 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/15 15:32:30 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "defs.hpp"
#include "channel.hpp"

class Server;

class Client{
public:
	Server* server;
	int fd;
	std::string nickname;
	std::string username;
	std::string realname;
	bool pass_ok;
	bool nick_ok;
	bool user_ok;
	bool registered;
	bool channel_ok;
	std::vector<std::string> lines_to_parse;
	std::string recv_buf;
	std::vector<std::string> send_buf;
	std::vector<std::string> joined_channels;

	explicit Client(int fd, Server* serv); // explicit force la creation d'une classe sous la forme: Client c(42)
	~Client();
	
	void process_data();

	void parse_lines();

	void add_to_send_buf(const std::string &data);

	void send_pending();

	void execute_command(const ParsedCommand &cmd);

	// void client_monitor()
};
