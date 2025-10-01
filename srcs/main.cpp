/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:31 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/01 15:57:07 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/struct_class.hpp"
#include "../incl/server.hpp"
#include <cstdlib>

int main(int ac, char **av)
{
  (void) av;
  if (ac != 3)
  {
    std::cerr << "you need to input 2 arguments (Port, Password)" << std::endl; //utilser std::cerr pour ecrire dans stderr en cas d'erreur ou de warning
    return 1;
  }
  start_server(atoi(av[1]), av[2]);
  return 0;
}
