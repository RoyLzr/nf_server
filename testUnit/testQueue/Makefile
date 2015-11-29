test:eventqueue.o icore.o main.o
	g++ -o test eventqueue.o icore.o main.o


eventqueue.o: ../../NfUnit/eventqueue.cpp
	g++ -g -c ../../NfUnit/eventqueue.cpp

icore.o: ../../interface/icore.cpp
	g++ -g -c ../../interface/icore.cpp

main.o: main.cpp
	g++ -g -c main.cpp

clean:
	rm -rf *.o
	rm -rf test

