#Version variables
VERSION = 0
PATCHLEVEL = 0
SUBLEVEL = 1

CC = g++
NAME = Server
DEBUG = -g3 -D_DEBUG
CFLAGS = -c -Wall -Wextra -Wconversion -O3 -std=c++11 

SRC = src/main.cpp src/Server.h src/Server.cpp
OBJ = $(SRC:.cpp=.o)

all: $(SRC) $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LFLAGS)

clean:
	rm $(OBJ) $(NAME)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)

