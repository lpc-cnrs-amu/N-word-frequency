CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC=calcul_occurences_thread calcul_occurences

all: $(EXEC)

calcul_occurences_thread: b_calcul_total_occurrences_thread.o util.o
	$(CC) -o calcul_occurences_thread b_calcul_total_occurrences_thread.o util.o $(LDFLAGS)
	
calcul_occurences: b_calcul_total_occurrences.o util.o
	$(CC) -o calcul_occurences b_calcul_total_occurrences.o util.o $(LDFLAGS)
	
b_calcul_total_occurrences_thread.o: b_calcul_total_occurrences_thread.cpp ../util/util.hpp ../util/OccurrencesSafe.hpp
	$(CC) -o b_calcul_total_occurrences_thread.o -c b_calcul_total_occurrences_thread.cpp $(CFLAGS)
	
b_calcul_total_occurrences.o: b_calcul_total_occurrences.cpp ../util/util.hpp ../util/OccurrencesSafe.hpp
	$(CC) -o b_calcul_total_occurrences.o -c b_calcul_total_occurrences.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)
