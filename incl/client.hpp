/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 17:54:15 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/13 18:01:21 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "defs.hpp"

class Server;

class Client {
public:
    int fd;
    std::string nickname;
    std::string username;
    std::string realname;
    bool registered;
    bool pass_ok;
    std::string recv_buf;
    std::string send_buf;
    std::set<std::string> joined_channels;

    explicit Client(int fd); // explicit force la creation d'une classe sous la forme: Client c(42)
    void receive_data(Server& serv);
    void send_pending();
};
