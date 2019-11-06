CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC=most_frequent_thread most_frequent

all: $(EXEC)

most_frequent_thread: e_take_n_most_frequencies_ngram_thread.o util.o fct_take_n_most_frequencies_ngram.o
	$(CC) -o most_frequent_thread e_take_n_most_frequencies_ngram_thread.o util.o fct_take_n_most_frequencies_ngram.o $(LDFLAGS)
	
most_frequent: e_take_n_most_frequencies_ngram.o util.o fct_take_n_most_frequencies_ngram.o
	$(CC) -o most_frequent e_take_n_most_frequencies_ngram.o util.o fct_take_n_most_frequencies_ngram.o  $(LDFLAGS)
	
e_take_n_most_frequencies_ngram_thread.o: e_take_n_most_frequencies_ngram_thread.cpp fct_take_n_most_frequencies_ngram.hpp
	$(CC) -o e_take_n_most_frequencies_ngram_thread.o -c e_take_n_most_frequencies_ngram_thread.cpp $(CFLAGS)
	
e_take_n_most_frequencies_ngram.o: e_take_n_most_frequencies_ngram.cpp fct_take_n_most_frequencies_ngram.hpp
	$(CC) -o e_take_n_most_frequencies_ngram.o -c e_take_n_most_frequencies_ngram.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
fct_take_n_most_frequencies_ngram.o: fct_take_n_most_frequencies_ngram.cpp fct_take_n_most_frequencies_ngram.hpp ../util/util.hpp
	$(CC) -o fct_take_n_most_frequencies_ngram.o -c fct_take_n_most_frequencies_ngram.cpp $(CFLAGS)
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)

