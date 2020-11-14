server : server.o
	g++ -o server server.o -pthread


server.o : server.cpp
	g++ -c -o server.o server.cpp


clean : 
	rm -f *.o
	rm -f $(TARGET)
