
CC=gcc

HRS= trietree.h memdbc.h
SCRS= trietree.c memdbc.c
OBJS= trietree.o memdbc.o

LDFLAGS=-g -L../utils/libs -L./ -L../utils/libs -L/usr/local/lib -lmemdbc -lstrutils -llogutils -lz -lpthread -lm
CFLAGS=-std=gnu99

CFLAGS += -g -Wall -O2 -I./ -I../utils/incs

ARC=libmemdbc.a

all: $(ARC) example1 example2

example1: example1.o $(ARC)
	$(CC) example1.o -o example1 $(LDFLAGS)

example2: example2.o $(ARC)
	$(CC) example2.o -o example2 $(LDFLAGS)

# example1.o: example1.c $(HRS)
#	$(CC) -c -o $@ $< $(CFLAGS)

$(ARC): $(OBJS)
	$(AR) -r $(ARC) $(OBJS)

%.o: %.c $(HRS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f example1 example2 $(ARC) $(OBJS) example1.o example2.o data*.txt \
	ascii1.txt data2.txt digital1.txt hex1.txt
