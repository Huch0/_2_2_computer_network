CC=gcc

all: calc.exe

calc.exe: calc.o core.o
	$(CC) -o calc.exe calc.o core.o


calc.o: calc.c
	$(CC) -c -o calc.o calc.c

core.o: core.c
	$(CC) -c -o core.o core.c

clean:
	rm -f calc.exe *.o