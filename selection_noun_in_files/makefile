CC=g++
CFLAGS=-W -Wall -std=c++11 -pedantic -O3 
LDFLAGS=-lpthread
EXEC=arbre

all: $(EXEC)

arbre: main.o Tokeniseur.o Noeud.o util.o
	$(CC) -o arbre main.o Tokeniseur.o Noeud.o util.o $(LDFLAGS)
	
main.o: main.cpp Tokeniseur.hpp
	$(CC) -o main.o -c main.cpp $(CFLAGS)
	
Tokeniseur.o: Tokeniseur.cpp Tokeniseur.hpp Noeud.hpp
	$(CC) -o Tokeniseur.o -c Tokeniseur.cpp $(CFLAGS)

Noeud.o: Noeud.cpp Noeud.hpp
	$(CC) -o Noeud.o -c Noeud.cpp $(CFLAGS)
	
util.o: ../util/util.cpp ../util/util.hpp ../util/QueueSafe.hpp
	$(CC) -o util.o -c ../util/util.cpp $(CFLAGS)
	
clean:
	rm -rf *.o
rmproper: clean
	rm -rf $(EXEC)
