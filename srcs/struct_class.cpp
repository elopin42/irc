/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct_class.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:24 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/09/30 19:07:53 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "struct_class.hpp"

//Client Class Definition


Client::Client(std::string password, std::string nickname, std::string username): _password(password), _nickname(nickname), _username(username)
{
    
}

Client::~Client()
{
    
}

