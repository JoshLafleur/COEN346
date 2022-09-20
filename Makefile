INC_DIR=./inc

CXX=g++
CXXFLAGS= -Wall -Wextra -Og -std=c++14 -I$(INC_DIR)

ASS1= src/ass1.cpp

ass1: $(ASS1)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f ass1
