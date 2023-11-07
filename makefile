CC = gcc
CFLAGS = -Wall -g

all: Sim

clean:
	rm -f *.o Sim

Sim: cacheSimulator.c
	$(CC) $(CFLAGS) cacheSimulator.c -o Sim -lm

