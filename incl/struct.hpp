#ifndef STRUCT_HPP
# define STRUCT_HPP

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

class client
{
  public:
	client();
	~client();

  private:
	std::string password;
	std::string nickname;
	std::string username;
};

client::client()
{
}

client::~client()
{
}

#endif // !STRUCT_HPP
