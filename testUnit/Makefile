server: net.o reactor.o main.o net_svr_cb.o asynLog.o event.o;
		g++ -o server net.o reactor.o main.o net_svr_cb.o asynLog.o event.o -lpthread

net.o:../net.h ../net.cpp
		g++ -g -c ../net.cpp

reactor.o: ../reactor.h ../reactor.cpp
		g++ -g -c ../reactor.cpp 

net_svr_cb.o: ../reactor.h ../reactor.cpp ../net_svr_cb.h ../net_svr_cb.cpp
		g++ -g -c ../net_svr_cb.cpp 

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
		
