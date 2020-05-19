include ../Makefile.common

CC=gcc
CFLAGS=-Wall -I../include -g
LDFLAGS= -L/usr/local/lib -lprussdrv -lpthread

all: $(EXEC)

$(EXEC): main.o display.o renderer.o
	$(CC) $^ -o $(EXEC) $(LDFLAGS)

main.o: main.c display.h
	$(CC) $(CFLAGS) -c main.c

display.o: display.c display.h
	$(CC) $(CFLAGS) -c display.c

renderer.o: renderer.c renderer.h
	$(CC) $(CFLAGS) -c renderer.c


install:
	ln -fs $(shell pwd)/$(EXEC) /usr/local/bin/


.PHONY: clean
