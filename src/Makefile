include ../Makefile.common

CC=gcc
CFLAGS=-Wall -I../include
LDFLAGS= -L/usr/local/lib -lprussdrv

all: $(EXEC)

$(EXEC): main.o display.o
	$(CC) $^ -o $(EXEC) $(LDFLAGS)

main.o: main.c display.h
	$(CC) $(CFLAGS) -c main.c

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c


install:
	ln -fs $(shell pwd)/$(EXEC) /usr/local/bin/

clean:
	rm -vf ../pru/*.bin *.o


.PHONY: clean
