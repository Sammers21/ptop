all: clean run

build: clean main.o

run: main.o
	./main.o

clean:
	rm -rf *.o

main.o: main.c uthash.h
	gcc -std=c11 -Wall -o main.o main.c

.PHONY: all run buid
