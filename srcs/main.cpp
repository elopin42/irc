/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yle-jaou <yle-jaou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 18:33:31 by yle-jaou          #+#    #+#             */
/*   Updated: 2025/10/22 22:35:21 by yle-jaou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incl/server.hpp"
#include <signal.h>

bool g_running = true;
Server *g_serv = NULL;

void handle_sigint(int)
{
  std::cout << "\n[INFO] Caught Ctrl+C → shutting down cleanly..." << std::endl;
  g_running = false;
  if (g_serv)
    delete_all(g_serv);
  exit(0);
}

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
    std::cerr << "[USAGE] ./ircserv <port> <password>" << std::endl;
    return 1;
  }

  if (!check_args(av))
    return 1;

  Server serv;
  g_serv = &serv;

  signal(SIGINT, handle_sigint);

  try
  {
    serv.run(av);
  }
  catch (const std::exception &e)
  {
    std::cerr << "[ERROR] " << e.what() << std::endl;
    delete_all(&serv);
    return 1;
  }

  delete_all(&serv);
  std::cout << "[INFO] Server exited cleanly." << std::endl;
  return 0;
}
