#Version variables
VERSION = 0
PATCHLEVEL = 0
SUBLEVEL = 1

CC = g++
NAME = Server
CFLAGS = -c -Wall -Wconversion -Wextra -Wno-missing-field-initializers -std=c++11 -DLDAP_DEPRECATED
LFLAGS= -lldap
ARCHIVE=$(NAME)-$(VERSION)

SRC = src/main.cpp src/Server.cpp
OBJ = $(SRC:.cpp=.o)

ifdef NDEBUG
CFLAGS+=-O3
else
CFLAGS+=-g -D_DEBUG
endif

all: $(SRC) $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LFLAGS)

clean:
	rm $(OBJ) $(NAME)

dist:
	cd ..; tar czf $(ARCHIVE).tar.gz $(NAME); mv $(ARCHIVE).tar.gz $(NAME)

distclean:
	rm $(ARCHIVE).tar.gz

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)
