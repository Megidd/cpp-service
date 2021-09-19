
CXX = g++-10
CXXFLAGS = -std=c++11 -I.. -Wall -Wextra -pthread

all: server

server : server.cc ../httplib.h Makefile
	$(CXX) -o server $(CXXFLAGS) server.cc

clean:
	rm server
