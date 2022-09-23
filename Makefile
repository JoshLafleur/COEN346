INC_DIR=./inc

CXX=g++
CXXFLAGS= -Wall -Wextra -g -O0 -std=c++17 -I$(INC_DIR) -lpthread -lc

ASS1= src/ass1.cpp

ass1: $(ASS1)
	$(CXX) $(CXXFLAGS) $^ -o $@

ass1-show-inputs: $(ASS1)
	$(CXX) $(CXXFLAGS) -DSHOW_INPUTS $^ -o ass1

clean:
	rm -f ass1
