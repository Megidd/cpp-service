
CXX = g++-10
CXXFLAGS = -std=c++11 -I.. -Wall -Wextra -pthread

all: test

test : test.cc httplib.h Makefile
	$(CXX) -o test $(CXXFLAGS) test.cc

clean:
	rm test
