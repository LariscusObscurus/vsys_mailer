CXX= g++
RM= rm -f
CPPFLAGS= -Isrc

MAINSOURCES= main.cpp server.cpp
CLIENTSOURCES= mainClient.cpp client.cpp
#OBJS=$(subst .cpp,$(SRCS))

all: clean server client

server:
	$(CXX) $(CPPFLAGS) $(MAINSOURCES) -o server
client: 
	$(CXX) $(CPPFLAGS) $(CLIENTSOURCES) -o client

clean:
	$(RM) server client
