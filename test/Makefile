server: net.o nf_server.o rapool.o nf_server_core.o nf_server_app.o server_simple.o lfpool.o configParser.o sapool.o queue.o memCache.o log.o
	rm -rf libs; 
	mkdir libs; 
	g++  -o server server_simple.o rapool.o asynLog.o queue.o net.o nf_server_app.o nf_server.o nf_server_core.o memCache.o lfpool.o configParser.o sapool.o -lpthread; 
	ls | grep '\.o' | xargs -I{} mv {} libs

net.o: ../net.h ../net.cpp
	g++ -g -c ../net.cpp

log.o: ../commonn/asynLog.cpp
	g++ -g -c ../commonn/asynLog.cpp
nf_server_app.o: ../nf_server_app.cpp
	g++ -g -c ../nf_server_app.cpp

queue.o:../commonn/queue.cpp
	g++ -g -c ../commonn/queue.cpp

lfpool.o:../nf_server.h
	g++ -g -c ../lfpool.cpp 

configParser.o:../commonn/configParser.cpp
	g++ -g -c ../commonn/configParser.cpp 

nf_server_core.o: ../nf_server_core.h ../nf_server_core.cpp ../net.h ../net.cpp
	g++ -g -c ../nf_server_core.cpp

nf_server.o: ../nf_server_core.h ../nf_server_core.cpp ../net.h ../net.cpp ../nf_server.h ../nf_server.cpp
	g++ -g -c ../nf_server.cpp

sapool.o:
	g++ -g -c ../sapool.cpp
rapool.o:
	g++ -g -c ../rapool.cpp

server_simple.o: ../nf_server_core.h ../nf_server_core.cpp ../net.h ../net.cpp ../nf_server.h ../nf_server.cpp server_simple.cpp
	g++ -g -c server_simple.cpp

memCache.o:../commonn/memCache.cpp
	g++ -g -c ../commonn/memCache.cpp


clean:
	ls | grep '\.o' | xargs rm -rf
	rm -rf server
	rm -rf libs

