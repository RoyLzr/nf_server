server: net.o reactor.o main.o nf_base_work.o asynLog.o event.o memCache.o;
		g++ -o server memCache.o net.o reactor.o main.o nf_base_work.o asynLog.o event.o -lpthread

net.o:../net.h ../net.cpp
		g++ -g -c ../net.cpp

memCache.o: ../commonn/memCache.h ../commonn/memCache.cpp
		g++ -g -c ../commonn/memCache.cpp

reactor.o: ../reactor.h ../reactor.cpp
		g++ -g -c ../reactor.cpp 

nf_base_work.o: ../reactor.h ../reactor.cpp ../nf_base_work.h ../nf_base_work.cpp
		g++ -g -c ../nf_base_work.cpp 

main.o: ../net.cpp ../reactor.cpp main.cpp
		g++ -g -c main.cpp -lpthread

asynLog.o: ../commonn/asynLog.cpp ../commonn/asynLog.h
		g++ -g -c ../commonn/asynLog.cpp

event.o:../event.h ../event.cpp
		g++ -g -c ../event.cpp

clean:
		ls | grep '\.o' |xargs rm -rf 
		ls | grep 'core*' |xargs rm -rf 
		rm -rf server
		
