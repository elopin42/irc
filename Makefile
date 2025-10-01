NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS =	srcs/main.cpp \
		srcs/struct_class.cpp \
		srcs/server.cpp \

OBJDIR = objs
OBJS = $(SRCS:srcs/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Compilation complete ✅"
	@$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: srcs/%.cpp
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning objects 🧹"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "Removing executable 🧼"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
