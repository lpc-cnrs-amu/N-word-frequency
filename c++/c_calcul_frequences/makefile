CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC=calcul_frequences_thread calcul_frequences

all: $(EXEC)

calcul_frequences_thread: c_calcul_frequences_thread.o util.o fct_calcul_frequences.o
	$(CC) -o calcul_frequences_thread c_calcul_frequences_thread.o util.o fct_calcul_frequences.o $(LDFLAGS)
	
calcul_frequences: c_calcul_frequences.o util.o fct_calcul_frequences.o
	$(CC) -o calcul_frequences c_calcul_frequences.o util.o fct_calcul_frequences.o $(LDFLAGS)
	
c_calcul_frequences_thread.o: c_calcul_frequences_thread.cpp fct_calcul_frequences.hpp 
	$(CC) -o c_calcul_frequences_thread.o -c c_calcul_frequences_thread.cpp $(CFLAGS)
	
c_calcul_frequences.o: c_calcul_frequences.cpp fct_calcul_frequences.hpp
	$(CC) -o c_calcul_frequences.o -c c_calcul_frequences.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
fct_calcul_frequences.o: fct_calcul_frequences.cpp fct_calcul_frequences.hpp ../util/util.hpp
	$(CC) -o fct_calcul_frequences.o -c fct_calcul_frequences.cpp $(CFLAGS)
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)

