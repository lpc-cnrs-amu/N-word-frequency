CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC= calcul_frequences_tag_grams

all: $(EXEC)

#calcul_frequences_thread: c_calcul_frequences_thread.o util.o
#	$(CC) -o calcul_frequences_thread c_calcul_frequences_thread.o util.o $(LDFLAGS)
	
calcul_frequences_tag_grams: d_calcul_frequences_tag_grams.o util.o fct_valid_lines.o
	$(CC) -o calcul_frequences_tag_grams d_calcul_frequences_tag_grams.o util.o fct_valid_lines.o $(LDFLAGS)
	
#c_calcul_frequences_thread.o: c_calcul_frequences_thread.cpp ../util/util.hpp 
#	$(CC) -o c_calcul_frequences_thread.o -c c_calcul_frequences_thread.cpp $(CFLAGS)
	
d_calcul_frequences_tag_grams.o: d_calcul_frequences_tag_grams.cpp ../util/util.hpp ../a_generate_files/fct_valid_lines.hpp ../util/Data.hpp
	$(CC) -o d_calcul_frequences_tag_grams.o -c d_calcul_frequences_tag_grams.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
fct_valid_lines.o: ../a_generate_files/fct_valid_lines.cpp ../a_generate_files/fct_valid_lines.hpp
	$(CC) -o fct_valid_lines.o -c ../a_generate_files/fct_valid_lines.cpp $(CFLAGS)	
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)

