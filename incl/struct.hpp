#ifndef STRUCT_HPP
# define STRUCT_HPP

#include <iostream>
#include <string>

struct rules {
  int port;
  std::string password;
};

class client {
public:
  client();
  ~client();

private:
  std::string password;
  std::string nickname;
  std::string username;
};

client::client() {
}

client::~client() {
}


#endif // !STRUCT_HPP


