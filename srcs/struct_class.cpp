/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct_class.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:24 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/01 15:57:12 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"

Client::Client(int fd, const std::string &ip, int port, int channel,
               const std::string &password,
               const std::string &nickname,
               const std::string &username)
    : channel(channel),
      fd(fd),
      ip(ip),
      port(port),
      _password(password),
      _nickname(nickname),
      _username(username),
      _input_buffer(""),
      _output_buffer("") {}


Client::~Client() {}
