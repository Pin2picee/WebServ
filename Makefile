MAKEFLAGS += -s  # global silence mode

C++				= c++
HDRS      = ./includes ./libs/includes
FLAGS			= -Wall -Wextra -Werror -std=c++98 -g $(addprefix -I, $(HDRS))

SRCS			=	main.cpp \
					srcs/utils.cpp \
					srcs/Client.cpp \
					srcs/Server.cpp \
					srcs/Config.cpp \
					srcs/Socket.cpp \
					srcs/Monitor.cpp \
					srcs/ResponseHandler.cpp
SRC_O			= $(SRCS:.cpp=.o)

NAME			= webserv

RM				= rm -f
CYAN			= "\033[36m"
RED				= "\033[31m"
GREEN			= "\033[32m"
RESET			= "\033[0m"

all: $(NAME)

# $(LIBFTCPP):
# 	@echo $(CYAN)"Building libftcpp library in submodule..."$(RESET)
# 	@$(MAKE) -C libs

$(NAME): $(SRC_O)
	@echo $(CYAN)"Building libftcpp library in submodule..."$(RESET)
	@$(MAKE) -C libs
	@echo $(CYAN)"Creation of $(NAME) executable..."$(RESET)
	@$(C++) $(FLAGS) $(SRC_O) libs/libftcpp.a -o $(NAME)
	@echo $(GREEN)"\"$(NAME)\" executable created !"$(RESET)

%.o: %.cpp
	@echo $(CYAN)"Compilation of $<..."$(RESET)
	@$(C++) $(FLAGS) -c $< -o $@

clean:
	@echo $(CYAN)"objets files suppression..."$(RESET)
	@find . -name "*.o" -type f -exec $(RM) {} \;
	@if [ -d libs ]; then $(MAKE) -C libs clean; fi

fclean: clean
	@echo $(CYAN)"executable suppression..."$(RESET)
	@$(RM) $(NAME)
	@if [ -d libs ]; then $(MAKE) -C libs fclean; fi

re: fclean all

.PHONY: all clean fclean re