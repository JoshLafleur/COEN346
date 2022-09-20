INC_DIR=./inc

CXX=g++
CXXFLAGS= -Wall -Wextra -g -O0 -std=c++17 -I$(INC_DIR) -lpthread

ASS1= src/ass1.cpp

ass1: $(ASS1)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f ass1
