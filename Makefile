INC_DIR=./inc

CXX=g++
CXXFLAGS= -Wall -Wextra -g -O0 -std=c++17 -I$(INC_DIR) -lpthread -lc

ASS1= src/ass1.cpp inc/ass1.h
ASS1_SUB= $(ASS1) Makefile Input.txt 1.pdf README.md

ASS2= src/ass2.cpp inc/ass2.h
ASS2_SUB= $(ASS1) Makefile Input.txt 2.pdf README.md

ASS3= src/ass3.cpp src/ass3-1.cpp inc/ass3.h inc/ass3-1.h
ASS3_SUB= $(ASS1) Makefile Input.txt 3.pdf README.md

ass1: $(ASS1)
	$(CXX) $(CXXFLAGS) $^ -o $@

ass1-test: $(ASS1)
	$(CXX) $(CXXFLAGS) -DTEST $^ -o ass1

ass1-submit: $(ASS1_SUB)
	7z a Lafleur_Joshua_40189389_ASS1.7z $^

ass2: $(ASS2)
	$(CXX) $(CXXFLAGS) $^ -o $@

ass2-test: $(ASS2)
	$(CXX) $(CXXFLAGS) -DTEST $^ -o ass2

ass2-submit: $(ASS2_SUB)
	tar cvf Lafleur_Joshua_40189389_ASS2.tar $^

ass3: $(ASS3)
	$(CXX) $(CXXFLAGS) $^ -o $@

ass3-test: $(ASS3)
	$(CXX) $(CXXFLAGS) -DTEST $^ -o ass3

ass3-submit: $(ASS3_SUB)
	tar cvf Lafleur_Joshua_40189389_ASS3.tar $^

clean:
	rm -f ass1
	rm -f ass2
	rm -f ass3
	rm -f Output.txt
	rm -f vm.txt
	rm -f *.7z
