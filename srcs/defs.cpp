#include "../incl/defs.hpp"

bool isValidNickname(const std::string &nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;

    char first = nick[0];
    if (!std::isalpha(first) &&
        first != '[' && first != ']' &&
        first != '\\' && first != '`' &&
        first != '_' && first != '^' &&
        first != '{' && first != '|' &&
        first != '}' && first != '-')
    {
        return false;
    }

    for (size_t i = 1; i < nick.length(); ++i)
    {
        char c = nick[i];
        if (!std::isalnum(c) &&
            c != '[' && c != ']' &&
            c != '\\' && c != '`' &&
            c != '_' && c != '^' &&
            c != '{' && c != '|' &&
            c != '}' && c != '-')
        {
            return false;
        }
    }

    return true;
}

bool isValidUsername(const std::string &username)
{
    if (username.empty())
        return false;
    if (username[0] == '-')
        return false;

    for (size_t i = 0; i < username.length(); ++i)
    {
        char c = username[i];
        if (!(std::isalnum(c) || c == '-' || c == '_' || c == '.'))
        {
            return false;
        }
    }

    return true;
}
