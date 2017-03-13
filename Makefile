all: main.cpp ./logger/logger.h
	g++ -std=c++11 -o main main.cpp -pthread

clean:
	rm -f main
