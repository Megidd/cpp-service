
CXX = g++-10
CXXFLAGS = -std=c++11 -I.. -Wall -Wextra -pthread

all: test-server test-client

test-server : test-server.cc httplib.h Makefile
	$(CXX) -o test-server $(CXXFLAGS) test-server.cc

test-client : test-client.cc httplib.h Makefile
	$(CXX) -o test-client $(CXXFLAGS) test-client.cc

clean:
	rm test-server test-client
