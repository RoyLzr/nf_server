test: configParser.o net.o nf_server.o memCache.o Buffer.o lineEvent.o eventqueue.o baseEvent.o icore.o syncReactor.o main.o ThreadManager.o extreactor.o asynLog.o 
	g++ -o test configParser.o nf_server.o net.o memCache.o Buffer.o lineEvent.o baseEvent.o syncReactor.o eventqueue.o icore.o main.o ThreadManager.o extreactor.o  asynLog.o -lpthread


nf_server.o:../../nf_server.cpp
	g++ -std=c++11 -g -c ../../nf_server.cpp

net.o: ../../net.cpp
	g++ -std=c++11 -g -c ../../net.cpp

memCache.o: ../../commonn/memCache.cpp
	g++ -std=c++11 -g -c ../../commonn/memCache.cpp
Buffer.o: ../../Buffer.cpp
	g++ -std=c++11 -g -c ../../Buffer.cpp

lineEvent.o: ../../NfUnit/lineEvent.cpp
	g++ -std=c++11 -g -c ../../NfUnit/lineEvent.cpp 

baseEvent.o: ../../NfUnit/baseEvent.cpp
	g++ -std=c++11 -g -c ../../NfUnit/baseEvent.cpp
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

