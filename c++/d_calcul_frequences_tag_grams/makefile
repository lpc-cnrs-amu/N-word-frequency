CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC=calcul_frequences_tag_grams calcul_frequences_tag_grams_thread

all: $(EXEC)

calcul_frequences_tag_grams_thread: d_calcul_frequences_tag_grams_thread.o util.o fct_valid_lines.o fct_calcul_frequences_tag_grams.o
	$(CC) -o calcul_frequences_tag_grams_thread d_calcul_frequences_tag_grams_thread.o util.o fct_valid_lines.o fct_calcul_frequences_tag_grams.o $(LDFLAGS)
	
calcul_frequences_tag_grams: d_calcul_frequences_tag_grams.o util.o fct_valid_lines.o fct_calcul_frequences_tag_grams.o
	$(CC) -o calcul_frequences_tag_grams d_calcul_frequences_tag_grams.o util.o fct_valid_lines.o fct_calcul_frequences_tag_grams.o $(LDFLAGS)
	
d_calcul_frequences_tag_grams_thread.o: d_calcul_frequences_tag_grams_thread.cpp fct_calcul_frequences_tag_grams.hpp
	$(CC) -o d_calcul_frequences_tag_grams_thread.o -c d_calcul_frequences_tag_grams_thread.cpp $(CFLAGS)
	
d_calcul_frequences_tag_grams.o: d_calcul_frequences_tag_grams.cpp fct_calcul_frequences_tag_grams.hpp
	$(CC) -o d_calcul_frequences_tag_grams.o -c d_calcul_frequences_tag_grams.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
fct_valid_lines.o: ../a_generate_files/fct_valid_lines.cpp ../a_generate_files/fct_valid_lines.hpp
	$(CC) -o fct_valid_lines.o -c ../a_generate_files/fct_valid_lines.cpp $(CFLAGS)	
	
fct_calcul_frequences_tag_grams.o: fct_calcul_frequences_tag_grams.cpp fct_calcul_frequences_tag_grams.hpp
	$(CC) -o fct_calcul_frequences_tag_grams.o -c fct_calcul_frequences_tag_grams.cpp $(CFLAGS)	
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)

