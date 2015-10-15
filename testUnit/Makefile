server: net.o reactor.o main.o;
		g++ -o server net.o reactor.o main.o

net.o:../net.h ../net.cpp
		g++ -g -c ../net.cpp

reactor.o: ../reactor.h ../reactor.cpp
		g++ -g -c ../reactor.cpp

main.o: ../net.cpp ../reactor.cpp main.cpp
		g++ -g -c main.cpp

clean:
		ls | grep '\.o' |xargs rm -rf 
		rm -rf server
		
