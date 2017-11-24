CC = gcc
CFLAGS = -g -c -std=c99
AR = ar -rc
RANLIB = ranlib


Target: create

my_pthread.o: my_pthread.c
	$(CC) $(CFLAGS) my_pthread.c

malloc.o: malloc.c
	$(CC) $(CFLAGS) malloc.c

create: malloc.o, my_pthread.o
	$(CC) -g malloc.o my_pthread.c -o mallocandpthread