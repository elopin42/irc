/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct_class.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:26 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/01 15:51:23 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRUCT_CLASS_HPP
# define STRUCT_CLASS_HPP

# include <iostream>
# include <string>

typedef struct rules
{
	int	port;
	std::string password;
}		t_rules;

typedef struct server
{
}		t_server;

class Client
{
  private:
	std::string _password; // un password est il necessaire ? "Vous devez pouvoir vous authentifier, définir un nickname, un username" que veut dire authentifier ?
	std::string _nickname;
	std::string _username;

  public:
	Client(std::string password, std::string nickname, std::string username);
	~Client();

};

#endif // !STRUCT_HPP
