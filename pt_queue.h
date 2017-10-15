#ifndef PT_QUEUE_H
#define PT_QUEUE_H

#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>

typedef uint my_pthread_t;

typedef struct threadControlBlock {
	/* add something here */
	my_pthread_t tid;
	int status;
	ucontext_t* context;
}tcb; 

typedef struct t_node{
	//weight and time values for priority queue and scan function
	float weight;
	uint64_t time;
	tcb* thread_block;
	struct t_node* next;
}t_node;

typedef struct pt_queue{  
    struct t_node* head,*tail;
    int length;
}pt_queue;

//merge sort functions
void mergeSort(t_node** pqueue);
struct t_node* sortedMerge(struct t_node* a, struct t_node* b);
void frontBackSplit(struct t_node* source, struct t_node** front, struct t_node** back);
void printQueue(struct pt_queue *queue);

//goes through pq and adjusts weight based on runtime
void scan(pt_queue* pqueue);

// define function for creating thread queue and node
struct pt_queue* createPt_queue();
struct t_node* createT_node();

//define function for inserting t_node into queue
void enqueue(struct pt_queue* queue, struct t_node* newProcess);

//define function for retrieving node from given queue
struct t_node* dequeue(struct pt_queue* queue);

#endif