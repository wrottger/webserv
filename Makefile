NAME = webserv
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -g -std=gnu++98 -Iincl -Wshadow

SCRS = $(wildcard src/*.cpp)
OBJS = $(addprefix objs/, $(notdir $(SCRS:.cpp=.o)))

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "\033[32m[✔] \033[0m\033[1;32mCreated:\033[0m $(NAME)"

objs/%.o: src/%.cpp
	@mkdir -p objs
	@$(CXX) $(CXXFLAGS) -c $< -o $@ 1>/dev/null

clean:
	@rm -f $(OBJS)
	@rm -rf objs

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all re clean fclean
