#include "../incl/struct.hpp"

int main(int ac, char **av)
{
  (void) av;
  if (ac != 3)
  {
    std::cout << "you need to input 2 arguments" << std::endl;
    return 1;
  }
  return 0;
}
