/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 17:54:15 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/14 14:08:57 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "defs.hpp"
#include "server.hpp"

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
    std::vector<std::string> lines_to_parse;
    std::string recv_buf;
    std::vector<std::string> send_buf;
    std::set<std::string> joined_channels;

    explicit Client(int fd, Server* serv); // explicit force la creation d'une classe sous la forme: Client c(42)
    ~Client();
    
    void process_data();

    void parse_lines();

    void send_pending();

    // void client_monitor();
};
