#ifndef MALLOC_H_
#define MALLOC_H_

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

/*
#define THREADREQ 0
#define LIBRARYREQ 1

#define malloc(x) 	myallocate(x, __FILE__, __LINE__,THREADREQ)
#define free(x)		mydeallocate(x, __FILE__, __LINE__,THREADREQ)
*/

static int getFreeIndex();
static int sGetFreeIndex();
void mallocInit();
void *myallocate(size_t size, char *file, size_t line, unsigned int requester);
void mydeallocate(void* ptr, char * file, size_t line, unsigned int requester);
void* shalloc(size_t size);
//void printError(char* error, char *file, size_t line);
#endif
