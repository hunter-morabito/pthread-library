CC = gcc
CFLAGS = -g -c -std=c99
AR = ar -rc
RANLIB = ranlib


Target: all

my_pthread.o: my_pthread.c my_pthread_t.h
	$(CC) $(CFLAGS) my_pthread.c

malloc.o: malloc.c malloc.h 
	$(CC) $(CFLAGS) malloc.c

all: malloc.o my_pthread.o
	$(CC) -g malloc.o my_pthread.c -o mallocandpthread