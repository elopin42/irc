
#ifndef DEFS_HPP
#define DEFS_HPP

#include <string>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdio>
#include <sys/epoll.h>
#include <sstream>
#include <algorithm>

#include <vector>
#include <set>
#include <map>

#define MAX_EVENTS 10

#define ERR_NOSUCHNICK       401  // No such nick
#define ERR_NOSUCHCHANNEL    403  // No such channel
#define ERR_CANNOTSENDTOCHAN 404  // Cannot send to channel
#define ERR_TOOMANYCHANNELS  405  // You have joined too many channels
#define ERR_NORECIPIENT      411  // No recipient given (PRIVMSG)
#define ERR_NOTEXTTOSEND     412  // No text to send
#define ERR_UNKNOWNCOMMAND   421  // Unknown command
#define ERR_NICKCOLLISION    436  // Nick collision

#define ERR_USERONCHANNEL 443

bool isValidNickname(const std::string &nick);
bool isValidUsername(const std::string &username);

#endif
