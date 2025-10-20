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

bool is_valid_channel_name(const std::string &name)
{
    if (name.empty() || name[0] != '#')
        return false;
    if (name.size() > 50)
        return false;
    for (size_t i = 1; i < name.size(); ++i)
    {
        if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
            return false;
    }
    return true;
}

std::vector<std::string> split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        // Trim leading whitespace
        size_t start = token.find_first_not_of(" \t\r\n");
        if (start != std::string::npos)
            token = token.substr(start);
        else
            token = "";
        
        // Trim trailing whitespace
        if (!token.empty())
        {
            size_t end = token.find_last_not_of(" \t\r\n");
            token = token.substr(0, end + 1);
        }
        
        if (!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}

