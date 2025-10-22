NAME = ircserv
CC = c++
CFLAGS = -g3 -Wall -Wextra -Werror -std=c++98

SRCS =	srcs/cmds/CAP.cpp		\
		srcs/cmds/INVITE.cpp	\
		srcs/cmds/JOIN.cpp		\
		srcs/cmds/KICK.cpp		\
		srcs/cmds/MODE.cpp		\
		srcs/cmds/NICK.cpp		\
		srcs/cmds/NOTICE.cpp	\
		srcs/cmds/PART.cpp		\
		srcs/cmds/PASS.cpp		\
		srcs/cmds/PING.cpp		\
		srcs/cmds/PONG.cpp		\
		srcs/cmds/PRIVMSG.cpp	\
		srcs/cmds/QUIT.cpp		\
		srcs/cmds/TOPIC.cpp		\
		srcs/cmds/USER.cpp		\
		srcs/cmds/WHO.cpp		\
		srcs/cmds/WHOIS.cpp		\
		srcs/bot.cpp			\
		srcs/channel.cpp		\
		srcs/client.cpp			\
		srcs/commands.cpp		\
		srcs/defs.cpp			\
		srcs/main.cpp			\
		srcs/server.cpp 		\

OBJDIR = objs
OBJS = $(SRCS:srcs/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Compilation complete ✅"
	@$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: srcs/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -I./incl -c $< -o $@

clean:
	@echo "Cleaning objects 🧹"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "Removing executable 🧼"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
