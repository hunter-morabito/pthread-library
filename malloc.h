#ifndef MALLOC_H_
#define MALLOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void mallocInit();
void *myallocate(size_t size, char *file, size_t line, unsigned int requester);
void mydeallocate(void* ptr, char * file, size_t line, unsigned int requester);
void* shalloc(size_t size);

#endif
