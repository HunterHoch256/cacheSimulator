CC = gcc
CFLAGS = -Wall -g

all: Sim.exe

clean:
	rm -f *.o Sim.exe

Sim.exe: cacheSimulator.c
	$(CC) $(CFLAGS) cacheSimulator.c -o Sim.exe -lm
