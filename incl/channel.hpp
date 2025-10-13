/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/13 18:00:46 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/13 18:01:02 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "defs.hpp"

class Client;

class Channel {
public:
    std::string name;
    std::string topic;
    std::set<Client*> members;

    explicit Channel(std::string n);
    void broadcast(const std::string& msg, Client* except = nullptr);
    void add_member(Client* c);
    void remove_member(Client* c);
};
