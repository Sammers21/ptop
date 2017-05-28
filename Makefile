
all: clean run

run: main.o
	./main.o

clean:
			rm -rf *.o
main.o: main.c
			gcc -Wall -o main.o main.c

.PHONY: all run
