all: clean Test

Test: Test.cpp 
	g++ -std=c++11 -O3 -march=native -Wall -o $@ $^ -lpthread
clean: 
	rm -rf Test
