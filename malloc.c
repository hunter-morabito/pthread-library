#include "malloc.h"
#include <stdint.h>

#define MEMSIZE 8000000

typedef unsigned char bool;

//this overhead is ~24 bytes
struct Page {
	bool isfree;		// 1 - yes, 0 - no
	size_t freeSpace;
	unsigned int ownerTread;
};

//this overhead is ~32 bytes
struct MemEntry {
	struct MemEntry *next, *prev;
	bool isfree;		// 1 - yes, 0 - no
	size_t size;		//size will NOT include the size of the MemEntry Overhead
};

static char memblock[MEMSIZE]; //big block of memory space

//should have a separate function for intializing
void mallocInit() {
	static struct Page * temp;
	static struct MemEntry * firstEntry;
	//create EVERY page and FIRST mementry overhead
	int i = 0;
	for (; i < MEMSIZE - (5*4096); i += 4096){
		temp = (struct Page*)(memblock + i);
		temp->isfree = 1;
		temp->freeSpace = 4096 - sizeof(struct Page) - sizeof(struct MemEntry);
		temp->ownerTread = 0;
		firstEntry = (struct MemEntry *)(memblock + i + sizeof(struct Page)); //static location of first mementry
		firstEntry->isfree = 1;
		firstEntry->size = temp->freeSpace;
		firstEntry->next = NULL;
		firstEntry->prev = NULL;
	}
}

// return a pointer to the memory buffer requested
void * myallocate(size_t size, char *file, size_t line, unsigned int requester) {
	if (size == 0 || size > 4096 - sizeof(struct MemEntry) - sizeof(struct Page)) {
		fprintf(stderr, "Unable to allocate this many bytes in FILE: '%s' on LINE: %zu\n", file, line);
		return 0;
	}

	//this will iterate over every page
	int i = 0;
	for (; i < MEMSIZE - (5*4096); i += 4096) {
		struct Page * pageptr = (struct Page*)(memblock + i);
		if (pageptr->ownerTread == requester || pageptr->isfree) {
			if (pageptr->freeSpace >= size) {
				struct MemEntry * memptr = (struct MemEntry *)(memblock + i + sizeof(struct Page)); //you see the static location used here again
				//this will iterate over every MemEntry in the Page
				for (; memptr != NULL; memptr = memptr->next) {
					if (memptr->isfree && memptr->size >= size) { //need to check if the entire page is free first
						if (pageptr->isfree){
							pageptr->ownerTread = requester;
							pageptr->isfree = 0;
						}
						pageptr->freeSpace -= size;
						if (pageptr->freeSpace < sizeof(struct MemEntry)) {
							memptr->next = NULL; //no room left for overhead just point it to NULL
						}
						else if (memptr->next != NULL) { //this is compilcated as we are checking the space in between two memory allocations now
							size_t freeSize = memptr->size - size;
							if (freeSize <= sizeof(struct MemEntry)) { //not enough space in between entries to create an overhead just allocate the rest of the space to this entry
								pageptr->freeSpace -= freeSize;
							}
							else {
								struct MemEntry * memptr_next = memptr->next;
								memptr->next = (struct MemEntry *)((char *)memptr + sizeof(struct MemEntry) + size); //create overhead for next memory entry and point to it
								memptr->next->isfree = 1;
								memptr->next->size = memptr->size - size - sizeof(struct MemEntry);
								memptr->next->prev = memptr;
								memptr->next->next = memptr_next;
								memptr_next->prev = memptr->next;
								pageptr->freeSpace -= sizeof(struct MemEntry);
							}
						}
						else { //in order to get an accurate representation of what free space is left we need to find how much space is left until we reach the next page
							memptr->next = (struct MemEntry *)((char *)memptr + sizeof(struct MemEntry) + size);
							uintptr_t nextpageadr = (uintptr_t)(pageptr + 4096);
							uintptr_t nextmemptr = (uintptr_t)(memptr->next + sizeof(struct MemEntry));
							uintptr_t diff = nextpageadr - nextmemptr;
							memptr->next->isfree = 1;
							pageptr->freeSpace -= sizeof(struct MemEntry);
							memptr->next->size = (unsigned int)diff;
							memptr->next->prev = memptr;
							memptr->next->next = NULL;
						}
						memptr->size = size;
						memptr->isfree = 0;
						return (char *)memptr + sizeof(struct MemEntry);
					}
				}
			}
		}
	}

	fprintf(stderr, "Insufficient memory space requested (bytes) in FILE: '%s' on LINE: %zu\n", file, line);
	return NULL;
}

// free a memory buffer pointed to by p
void mydeallocate(void * memlocation, char *file, size_t line, unsigned int requester)
{
	if (memlocation == NULL) {
		fprintf(stderr, "Pointer is NULL in file, free failed in FILE: '%s' on LINE: %zu\n", file, line);
		return;
	}

	struct MemEntry * memptr = (struct MemEntry*)((char*)memlocation - sizeof(struct MemEntry));
	//find the page of the MemEntry
	struct MemEntry * tempptr = memptr;
	while (tempptr->prev != NULL) {
		//this will put our pointer at the first MemEntry
		tempptr = tempptr->prev;
	}
	struct Page * pageptr = (struct Page *)((char *)tempptr - sizeof(struct Page));

	if (memptr->isfree == 0) {
		struct MemEntry *prev, *next;
		prev = memptr->prev;
		next = memptr->next;
		if (prev != NULL && prev->isfree) { //combine the previous and current blocks
			if (next != NULL && next->isfree) { //combine all three
				prev->size += memptr->size + next->size + 2 * sizeof(struct MemEntry);
				prev->next = next->next;
				next->next->prev = prev;
				pageptr->freeSpace += memptr->size + 2 * sizeof(struct MemEntry);
			}
			else {
				prev->size += memptr->size + sizeof(struct MemEntry);
				prev->next = next;
				next->prev = prev;
				pageptr->freeSpace += memptr->size + sizeof(struct MemEntry);
			}
		}
		else if (next != NULL && next->isfree) {
			memptr->size += next->size + sizeof(struct MemEntry);
			memptr->next = next->next;
			next->next->prev = memptr;
			memptr->isfree = 1;
			pageptr->freeSpace += memptr->size + sizeof(struct MemEntry);
		} 
		else {
			memptr->isfree = 1;
			pageptr->freeSpace += memptr->size;
		}
	}
}


// return a pointer to the memory buffer requested
void* shalloc(size_t size) {
	if (size == 0 || size > 4096 - sizeof(struct MemEntry) - sizeof(struct Page)) {
		fprintf(stderr, "Unable to allocate this many bytes in shalloc.\n");
		return 0;
	}

	//this will iterate over every page
	int i = MEMSIZE - (4*4096);
	for (; i < MEMSIZE; i += 4096) {
		struct Page * pageptr = (struct Page*)(memblock + i);
		if (pageptr->isfree) {
			if (pageptr->freeSpace >= size) {
				struct MemEntry * memptr = (struct MemEntry *)(memblock + i + sizeof(struct Page)); //you see the static location used here again
				//this will iterate over every MemEntry in the Page


				for (; memptr != NULL; memptr = memptr->next) {
					if (memptr->isfree && memptr->size >= size) { //need to check if the entire page is free first
						if (pageptr->isfree){
							pageptr->ownerTread = 0;
							pageptr->isfree = 0;
						}
						pageptr->freeSpace -= size;
						if (pageptr->freeSpace < sizeof(struct MemEntry)) {
							memptr->next = NULL; //no room left for overhead just point it to NULL
						}
						else if (memptr->next != NULL) { //this is compilcated as we are checking the space in between two memory allocations now
							size_t freeSize = memptr->size - size;
							if (freeSize <= sizeof(struct MemEntry)) { //not enough space in between entries to create an overhead just allocate the rest of the space to this entry
								pageptr->freeSpace -= freeSize;
							}
							else {
								struct MemEntry * memptr_next = memptr->next;
								memptr->next = (struct MemEntry *)((char *)memptr + sizeof(struct MemEntry) + size); //create overhead for next memory entry and point to it
								memptr->next->isfree = 1;
								memptr->next->size = memptr->size - size - sizeof(struct MemEntry);
								memptr->next->prev = memptr;
								memptr->next->next = memptr_next;
								memptr_next->prev = memptr->next;
								pageptr->freeSpace -= sizeof(struct MemEntry);
							}
						}
						else { //in order to get an accurate representation of what free space is left we need to find how much space is left until we reach the next page
							memptr->next = (struct MemEntry *)((char *)memptr + sizeof(struct MemEntry) + size);
							uintptr_t nextpageadr = (uintptr_t)(pageptr + 4096);
							uintptr_t nextmemptr = (uintptr_t)(memptr->next + sizeof(struct MemEntry));
							uintptr_t diff = nextpageadr - nextmemptr;
							memptr->next->isfree = 1;
							pageptr->freeSpace -= sizeof(struct MemEntry);
							memptr->next->size = (unsigned int)diff;
							memptr->next->prev = memptr;
							memptr->next->next = NULL;
						}
						memptr->size = size;
						memptr->isfree = 0;
						return (char *)memptr + sizeof(struct MemEntry);
					}
				}


			}
		}
	}

	fprintf(stderr, "Insufficient memory space requested (bytes) in shalloc.\n");
	return NULL;
}

/*
int main(){
	mallocInit();
	int* a = (int*)malloc(400);
	int* b = (int*)malloc(10);
	int* c = (int *)malloc(30);
	a[0] = 5;
	b[0]= 20;
	c[0] = 70;

	printf("Page: %lu MemEntry: %lu\n", sizeof(struct Page), sizeof(struct MemEntry));
	printf("a: %d\n", memblock[sizeof(struct Page) + sizeof(struct MemEntry)]);
	printf("b: %d\n", memblock[sizeof(struct Page) + sizeof(struct MemEntry) + 400 + sizeof(struct MemEntry)]);
	printf("c: %d\n", memblock[sizeof(struct Page) + sizeof(struct MemEntry) + 400 + sizeof(struct MemEntry) + 10 + sizeof(struct MemEntry)]);
	free(a);
	int* d = (int*) malloc(20);
	d[0] = 2;
	printf("a or d: %d\n", memblock[sizeof(struct Page) + sizeof(struct MemEntry)]);
	int* e = (int*) malloc(400);
	e[0] = 44;
	printf("e: %d\n", memblock[sizeof(struct Page) + sizeof(struct MemEntry) + 400 + sizeof(struct MemEntry) + 10 + sizeof(struct MemEntry) + 30 + sizeof(struct MemEntry)]);
	int* f = (int *)malloc(100); //should be between d and b
	f[0] = 1024;
	printf("f: %d\n", f[0]); //24 + 32 + 20 + 32


	free(a);
	free(b);
	free(c);
	free(c);
	return 0;
}*/
