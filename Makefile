MAKEFLAGS += -s  # global silence mode

C++				= c++
HDRS			= ./includes
FLAGS			= -Wall -Wextra -Werror -pedantic -Wpedantic -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 -g -I $(HDRS)

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

# =======================
# Tests HTTP avec curl
# =======================

IP      = 127.0.0.1
PORT1  = 8080
PORT2  = 8081

test:
	@echo $(CYAN)"Testing server on port $(PORT1)..."$(RESET)
	@curl --resolve localhost:$(PORT1):$(IP) http://localhost:$(PORT1)/ || true
	@echo
	@echo $(CYAN)"Testing server on port $(PORT2)..."$(RESET)
	@curl --resolve localhost:$(PORT2):$(IP) http://localhost:$(PORT2)/ || true

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

.PHONY: all clean fclean re test