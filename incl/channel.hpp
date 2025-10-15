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

#include <string>
#include <vector>
#include <set>

struct ParsedCommand {
    std::string cmd;
    std::vector<std::string> args;
};

class Channel {
public:
    std::string name;
    std::string topic;
    std::set<std::string> users; // nicknames of users in the channel
    std::set<std::string> operators; // nicknames of operators

    Channel(const std::string& name);
    ~Channel();

    void add_user(const std::string& nickname);
    void remove_user(const std::string& nickname);
    void set_topic(const std::string& new_topic);
    bool is_operator(const std::string& nickname) const;
    void add_operator(const std::string& nickname);
    void remove_operator(const std::string& nickname);
};

#endif