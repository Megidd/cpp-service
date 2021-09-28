
CXX = g++-10
CXXFLAGS = -std=c++11 -I.. -Wall -Wextra -pthread

all: server client

server : server.cc httplib.h Makefile
	$(CXX) -o server $(CXXFLAGS) server.cc

client : client.cc httplib.h Makefile
	$(CXX) -o client $(CXXFLAGS) client.cc

clean:
	rm server client
