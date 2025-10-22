MAKEFLAGS += -s  # global silence mode

C++				= c++
HDRS			= ./includes
FLAGS			= -Wall -Wextra -Werror -std=c++98 -g -I $(HDRS)

SRCS			=	main.cpp \
					srcs/utils.cpp \
					srcs/Server.cpp \
					srcs/Config.cpp \
					srcs/Response.cpp 
SRC_O			= $(SRCS:.cpp=.o)

NAME			= webserv

RM				= rm -f
CYAN			= "\033[36m"
RED				= "\033[31m"
GREEN			= "\033[32m"
RESET			= "\033[0m"

all: $(NAME)

$(NAME): $(SRC_O)
	@echo $(CYAN)"Creation of $(NAME) executable..."$(RESET)
	@$(C++) $(FLAGS) $(SRC_O) -o $(NAME)
	@echo $(GREEN) "\"$(NAME)\" executable created !"$(RESET)

%.o: %.cpp
	@echo $(CYAN)"Compilation of $<..."$(RESET)
	@$(C++) $(FLAGS) -c $< -o $@

clean:
	@echo $(CYAN)"objets files suppression..."$(RESET)
	@find . -name "*.o" -type f -exec $(RM) {} \;

fclean: clean
	@echo $(CYAN)"executable suppression..."$(RESET)
	@$(RM) $(NAME) 

re: fclean all

.PHONY: all clean fclean re