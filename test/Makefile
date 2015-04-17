server: net.o nf_server.o nf_server_core.o server_simple.o lfpool.o 
	rm -rf libs; 
	mkdir libs; 
	g++  -o server server_simple.o net.o nf_server.o nf_server_core.o lfpool.o -lpthread; 
	ls | grep '.o' | xargs -I{} mv {} libs
net.o: ../net.h ../net.cpp
	g++ -g -c ../net.cpp
lfpool.o:../nf_server.h
	g++ -g -c ../lfpool.cpp 
nf_server_core.o: ../nf_server_core.h ../nf_server_core.cpp ../net.h ../net.cpp
	g++ -g -c ../nf_server_core.cpp
nf_server.o: ../nf_server_core.h ../nf_server_core.cpp ../net.h ../net.cpp ../nf_server.h ../nf_server.cpp
	g++ -g -c ../nf_server.cpp
server_simple.o: ../nf_server_core.h ../nf_server_core.cpp ../net.h ../net.cpp ../nf_server.h ../nf_server.cpp server_simple.cpp
	g++ -g -c server_simple.cpp

clean:
	ls | grep '.o' | xargs rm -rf
	rm -rf server
	rm -rf libs

