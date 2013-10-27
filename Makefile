#Version variables
VERSION = 0
PATCHLEVEL = 0
SUBLEVEL = 1

CC = g++
SERV = Server
CLIENT = Client

CFLAGS = -c -Wall -Wconversion -Wextra -Wno-missing-field-initializers -std=c++11 -DLDAP_DEPRECATED
LFLAGS= -lldap -llber

SERVSRC = src/main.cpp src/Server.cpp
CLIENTSRC = src/client.cpp src/mainClient.cpp

OBJSERV = $(SERVSRC:.cpp=.o)
OBJCLIENT = $(CLIENTSRC:.cpp=.o)

ifdef NDEBUG
CFLAGS+=-O3
else
CFLAGS+=-g -D_DEBUG
endif

all: $(SERV) $(CLIENT)

$(CLIENT): $(OBJCLIENT)
	$(CC) $(OBJCLIENT) -o $@

$(SERV): $(OBJSERV)
	$(CC) $(OBJSERV) -o $@ $(LFLAGS)

clean:
	rm $(OBJSERV) $(SERV)
	rm $(OBJCLIENT) $(CLIENT)

%.o: %.cpp 
	$(CC) -c $< -o $@ $(CFLAGS)
