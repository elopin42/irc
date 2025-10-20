/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:31 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/20 19:42:32 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"

bool check_args(char **av)
{
  int port = atoi(av[1]);
  if (port < 1024 || port > 65535)
  {
    std::cerr << "invalid port number, need to be between 1024 and 65535" << std::endl;
    return false;
  }
  std::string pass = av[2];
  if (pass.size() < 4)
  {
    std::cerr << "invalid password length, minimum is 4" << std::endl;
    return false;
  }
  for (size_t i = 0; av[2][i]; i++)
    if (av[2][i] < 32 || av[2][i] > 126)
    {
      std::cerr << "invalid password characters, valid ones are from ascii 32 to ascii 126" << std::endl;
      return false;
    }
  return true;
}

int main(int ac, char **av)
{
  if (ac != 3)
  {
    std::cerr << "you need to input 2 arguments (Port, Password)" << std::endl;
    return 1;
  }
  if (!check_args(av))
    return 1;
  Server serv;
  try
  {
    serv.run(av);
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    //Liberer Lacrim, et la map des channels et des clients ca peut arriver en cas de fail a un system call (souvent ce connard de recv de merde)
    return 1;
  }
}
