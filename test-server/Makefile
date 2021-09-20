
CXX = g++-10
CXXFLAGS = -std=c++11 -I.. -Wall -Wextra -pthread

all: test-server

test-server : test-server.cc httplib.h Makefile
	$(CXX) -o test-server $(CXXFLAGS) test-server.cc

clean:
	rm test-server
