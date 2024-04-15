NAME = webserv
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -g -std=gnu++98 -Iincl -Wshadow

CLT_NAME = client

SRC := 	main.cpp

CLT_SRC := 	browser.cpp

HEADER :=	

OBJ := $(SRC:.cpp=.o)

#CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -g
#CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address

.PHONY: all bonus clean fclean re

all:$(NAME) $(CLT_NAME)

$(NAME): $(OBJ)
	c++ $(CXXFLAGS) $(OBJ) -o $(NAME)
$(OBJ): $(SRC) $(HEADER)
	c++ -c $(CXXFLAGS) $(SRC)

CLT_OBJ := $(CLT_SRC:.cpp=.o)


$(CLT_NAME): $(CLT_OBJ)
	c++ $(CXXFLAGS) $(CLT_OBJ) -o $(CLT_NAME)
$(CLT_OBJ): $(CLT_SRC) $(CLT_HEADER)
	c++ -c $(CXXFLAGS) $(CLT_SRC)

	.
clean:
	rm -f $(OBJ) $(CLT_OBJ) 
fclean: clean
	rm -f $(NAME) $(CLT_NAME) 
re: fclean all