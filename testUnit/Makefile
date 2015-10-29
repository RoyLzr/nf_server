server: ThreadPool.o net.o reactor.o main.o nf_base_work.o asynLog.o event.o memCache.o Buffer.o;
		g++ -o server ThreadPool.o memCache.o net.o reactor.o main.o nf_base_work.o asynLog.o Buffer.o event.o -lpthread


Buffer.o: ../Buffer.h ../Buffer.cpp
		g++ -g -c ../Buffer.cpp

net.o:../net.h ../net.cpp
		g++ -g -c ../net.cpp
ThreadPool.o:../commonn/ThreadPool.h ../commonn/ThreadPool.cpp
		g++ -g -c ../commonn/ThreadPool.cpp

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
		
