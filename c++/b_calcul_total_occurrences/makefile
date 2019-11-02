CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC=calcul_occurences_thread calcul_occurences

all: $(EXEC)

calcul_occurences_thread: b_calcul_total_occurrences_thread.o util.o fct_calcul_total_occurrences.o
	$(CC) -o calcul_occurences_thread b_calcul_total_occurrences_thread.o util.o fct_calcul_total_occurrences.o $(LDFLAGS)
	
calcul_occurences: b_calcul_total_occurrences.o util.o fct_calcul_total_occurrences.o
	$(CC) -o calcul_occurences b_calcul_total_occurrences.o util.o fct_calcul_total_occurrences.o  $(LDFLAGS)
	
b_calcul_total_occurrences_thread.o: b_calcul_total_occurrences_thread.cpp ../util/util.hpp ../util/OccurrencesSafe.hpp
	$(CC) -o b_calcul_total_occurrences_thread.o -c b_calcul_total_occurrences_thread.cpp $(CFLAGS)
	
b_calcul_total_occurrences.o: b_calcul_total_occurrences.cpp ../util/util.hpp ../util/OccurrencesSafe.hpp
	$(CC) -o b_calcul_total_occurrences.o -c b_calcul_total_occurrences.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
fct_calcul_total_occurrences.o: fct_calcul_total_occurrences.cpp fct_calcul_total_occurrences.hpp ../util/util.hpp
	$(CC) -o fct_calcul_total_occurrences.o -c fct_calcul_total_occurrences.cpp $(CFLAGS)
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)
