test:eventqueue.o icore.o syncReactor.o main.o ThreadManager.o extreactor.o configParser.o asynLog.o
	g++ -o test syncReactor.o eventqueue.o icore.o main.o ThreadManager.o extreactor.o configParser.o asynLog.o -lpthread

configParser.o: ../../commonn/configParser.cpp
	g++  -std=c++11 -g -c ../../commonn/configParser.cpp
asynLog.o: ../../commonn/asynLog.cpp
	g++  -std=c++11 -g -c ../../commonn/asynLog.cpp

extreactor.o: ../../NfUnit/extreactor.cpp
	g++ -std=c++11 -g -c ../../NfUnit/extreactor.cpp

syncReactor.o: ../../NfUnit/syncReactor.cpp
	g++ -std=c++11 -g -c ../../NfUnit/syncReactor.cpp

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
	rm -rf core*
	rm -rf test

