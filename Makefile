all:
	g++ -std=c++11 -pthread -L. -Wall -o cam.o cam.cpp `pkg-config --cflags --libs opencv`
