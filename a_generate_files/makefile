CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lz -lpthread
EXEC=generate_treated_files generate_treated_files_thread

all: $(EXEC)

generate_treated_files_thread: a_generate_files_thread.o util.o fct_valid_lines.o fct_generate_files.o
	$(CC) -o generate_treated_files_thread a_generate_files_thread.o util.o fct_valid_lines.o fct_generate_files.o $(LDFLAGS)
	
generate_treated_files: a_generate_files.o util.o fct_valid_lines.o fct_generate_files.o
	$(CC) -o generate_treated_files a_generate_files.o util.o fct_valid_lines.o fct_generate_files.o $(LDFLAGS)
	
a_generate_files_thread.o: a_generate_files_thread.cpp ../util/util.hpp fct_valid_lines.hpp fct_generate_files.hpp
	$(CC) -o a_generate_files_thread.o -c a_generate_files_thread.cpp $(CFLAGS)
	
a_generate_files.o: a_generate_files.cpp ../util/util.hpp fct_valid_lines.hpp fct_generate_files.hpp
	$(CC) -o a_generate_files.o -c a_generate_files.cpp $(CFLAGS)

util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
fct_valid_lines.o: fct_valid_lines.cpp fct_valid_lines.hpp
	$(CC) -o fct_valid_lines.o -c fct_valid_lines.cpp $(CFLAGS)
	
fct_generate_files.o: fct_generate_files.cpp fct_generate_files.hpp ../util/util.hpp
	$(CC) -o fct_generate_files.o -c fct_generate_files.cpp $(CFLAGS)
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)

