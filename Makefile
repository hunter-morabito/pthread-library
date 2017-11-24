CC = gcc
CFLAGS = -g -c -std=c99
AR = ar -rc
RANLIB = ranlib


Target: my_pthread.c

my_pthread.o:
	$(CC) $(CFLAGS) my_pthread.c

malloc.o:
	$(CC) $(CFLAGS) malloc.c

my_pthread.c:
	$(CC) -g malloc.o my_pthread.c -o mallocandpthread