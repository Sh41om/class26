client : client.o
	g++ -o client client.o -pthread


client.o : client.cpp
	g++ -c -o client.o client.cpp -pthread


clean : 
	rm -f *.o
	rm -f $(TARGET)
