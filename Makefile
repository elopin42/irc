NAME    = ircserv
CXX     = c++
FLAGS   = -Wall -Wextra -Werror -std=c++98

SRCSF   = main.cpp \
					server.cpp
SRCDIR  = srcs/
OBJDIR  = objs/

SRCS    = $(addprefix $(SRCDIR), $(SRCSF))
OBJS    = $(addprefix $(OBJDIR), $(SRCSF:.cpp=.o))

# Couleurs
GREEN   = \033[1;32m
RED     = \033[1;31m
PINK    = \033[1;35m
RESET   = \033[0m

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS)
	@echo "$(PINK)|$(RESET)"
	@$(CXX) $(FLAGS) -o $(NAME) $(OBJS)
	@echo "$(GREEN)✔ irc ready$(RESET)"

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@echo "$(PINK)Compiling $< ...$(RESET)"
	@$(CXX) $(FLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)
	@echo "$(PINK)Creating obj directory...$(RESET)"

clean:
	@rm -rf $(OBJDIR)
	@echo "$(RED)✘ objects deleted$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)✘ binary deleted$(RESET)"

re: fclean all

.PHONY: all clean fclean re
