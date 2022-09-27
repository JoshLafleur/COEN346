INC_DIR=./inc

CXX=g++
CXXFLAGS= -Wall -Wextra -g -O0 -std=c++17 -I$(INC_DIR) -lpthread -lc

ASS1= src/ass1.cpp inc/ass1.h
ASS1_SUB= $(ASS1) Makefile

ass1: $(ASS1)
	$(CXX) $(CXXFLAGS) $^ -o $@

ass1-test: $(ASS1)
	$(CXX) $(CXXFLAGS) -DTEST $^ -o ass1

ass1-submit: $(ASS1_SUB)
	7z a Lafleur_Joshua_40189389_ASS1.7z $^

clean:
	rm -f ass1
	rm -f Output.txt
	rm -f *.7z
