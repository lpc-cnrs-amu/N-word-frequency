CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC=calcul_frequences_thread calcul_frequences

all: $(EXEC)

calcul_frequences_thread: c_calcul_frequences_thread.o util.o
	$(CC) -o calcul_frequences_thread c_calcul_frequences_thread.o util.o $(LDFLAGS)
	
calcul_frequences: c_calcul_frequences.o util.o
	$(CC) -o calcul_frequences c_calcul_frequences.o util.o $(LDFLAGS)
	
c_calcul_frequences_thread.o: c_calcul_frequences_thread.cpp ../util/util.hpp 
	$(CC) -o c_calcul_frequences_thread.o -c c_calcul_frequences_thread.cpp $(CFLAGS)
	
c_calcul_frequences.o: c_calcul_frequences.cpp ../util/util.hpp 
	$(CC) -o c_calcul_frequences.o -c c_calcul_frequences.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)

