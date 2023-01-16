
CC=gcc

HRS= trietree.h memdbc.h
SCRS= trietree.c memdbc.c
OBJS= trietree.o memdbc.o

LDFLAGS=-g -L../utils/libs -L./ -L../utils/libs -L/usr/local/lib -lstrutils -llogutils -lz -lpthread -lm
CFLAGS=-std=gnu99

CFLAGS += -g -Wall -O2 -I./ -I../utils/incs

all: example example2

example: example.o $(OBJS)
	$(CC) example.o $(OBJS) -o example $(LDFLAGS)

example2: example2.o $(OBJS)
	$(CC) example2.o $(OBJS) -o example2 $(LDFLAGS)

example.o: example.c $(HRS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.c $(HRS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f example example2 $(OBJS) example.o example2.o
