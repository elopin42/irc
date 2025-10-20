/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 10:00:00 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/15 10:00:00 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "defs.hpp"
#include "../incl/server.hpp"

class Channel {
public:
    Server *serv;

    std::string name;
    std::string topic;
    std::vector<std::string> users; // nicknames of users in the channel
    std::vector<std::string> operators; // nicknames of operators

    int limit_user;
    Channel(const std::string& name, Server *serv);
    ~Channel();

    void add_user(const std::string& nickname);
    void remove_user(const std::string& nickname);

    bool is_user(const std::string &nickname);

    void set_topic(const std::string& new_topic);
    bool is_operator(const std::string& nickname) const;
    void add_operator(const Client& setter, const std::string& nickname);
    void remove_operator(const std::string& nickname);
    void broadcast_message(const std::string &msg, const std::string &exclude_nick);
};

#endif
