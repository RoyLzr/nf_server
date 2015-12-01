test:eventqueue.o icore.o main.o ThreadManager.o
	g++ -o test eventqueue.o icore.o main.o ThreadManager.o -lpthread


eventqueue.o: ../../NfUnit/eventqueue.cpp
	g++ -std=c++11 -g -c ../../NfUnit/eventqueue.cpp

icore.o: ../../interface/icore.cpp
	g++ -std=c++11 -g -c ../../interface/icore.cpp

main.o: main.cpp
	g++  -std=c++11 -g -c main.cpp
ThreadManager.o: ../../commonn/ThreadManager.cpp
	g++  -std=c++11 -g -c ../../commonn/ThreadManager.cpp

clean:
	rm -rf *.o
	rm -rf test

