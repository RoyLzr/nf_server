server: net.o reactor.o main.o net_svr_cb.o;
		g++ -o server net.o reactor.o main.o net_svr_cb.o

net.o:../net.h ../net.cpp
		g++ -g -c ../net.cpp

reactor.o: ../reactor.h ../reactor.cpp
		g++ -g -c ../reactor.cpp -lpthread

net_svr_cb.o: ../reactor.h ../reactor.cpp ../net_svr_cb.h ../net_svr_cb.cpp
		g++ -g -c ../net_svr_cb.cpp -lpthread

main.o: ../net.cpp ../reactor.cpp main.cpp
		g++ -g -c main.cpp -lpthread

clean:
		ls | grep '\.o' |xargs rm -rf 
		ls | grep 'core*' |xargs rm -rf 
		rm -rf server
		
