#include "pt_queue.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint64_t getTimeStamp(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

struct t_node* createT_node(tcb* threadblock){
	//create new threadNode
	struct t_node* newNode = (struct t_node*)malloc(sizeof(struct t_node));
	//set attributes of t_node
	newNode->thread_block = threadblock;
	//have the pointer point to the thread location
	newNode->next = NULL;
	//set new process to highest priority
	newNode->weight = 0;
	//set time to the time it was initialized
	newNode->time = getTimeStamp();
	//return new node
	return newNode;
}

struct pt_queue* createPt_queue(){
	//allocate memory
	struct pt_queue* queue = (struct pt_queue*)malloc(sizeof(struct pt_queue));
	//init head and tail
    queue->head = NULL;
    queue->tail = NULL;
	//init length
	queue->length = 0;
	return queue;
}

void enqueue(struct pt_queue* pqueue, struct t_node* newProcess){
	//empty check
	if(pqueue->length == 0){
        pqueue->head = newProcess;
        pqueue->tail = newProcess;
        pqueue->length++;
		return;
	}
	//add node to front of queue since new processes have highest priority
	newProcess->next = pqueue->head;
	pqueue->head = newProcess;
	//increment number of threads in the queue
	pqueue->length++;
}

//DEPRECATED YOU SHOULD NOT CREATE AND ENQUEUE AT SAME TIME
/*
void enqueue(struct pt_queue* pqueue, tcb* threadBlock){
    struct t_node* temp = createT_node(threadBlock);
	//empty check
	if(pqueue->length == 0){
        pqueue->head = temp;
        pqueue->tail = temp;
        pqueue->length++;
		return;
	}
	//add node to front of queue since new processes have highest priority
	temp->next = pqueue->head;
	pqueue->head = temp;
	//increment number of threads in the queue
	pqueue->length++;
}*/

struct t_node* dequeue(struct pt_queue* pqueue){
	if(pqueue->length == 0){
		return NULL;
	}
	//hold onto previous head
	struct t_node* temp = pqueue->head;
	//update the head node to the next node
	pqueue->head = pqueue->head->next;
	//decrement the number of threadas you have in queue
	pqueue->length--;
	//if front is now NULL, also update rear to NULL
	if(pqueue->length == 0)
		pqueue->tail = NULL;
	//return thread
	return temp;
}

//merge sort is the preferred sort method for linked lists
void mergeSort(struct t_node** pqueue)
{
  struct t_node* head = *pqueue;
  struct t_node* a;
  struct t_node* b;
 
  /* Base case -- length 0 or 1 */
  if ((head == NULL) || (head->next == NULL))
  {
    return;
  }
 
  /* Split head into 'a' and 'b' sublists */
  frontBackSplit(head, &a, &b); 
 
  /* Recursively sort the sublists */
  mergeSort(&a);
  mergeSort(&b);
 
  /* answer = merge the two sorted lists together */
  *pqueue = sortedMerge(a, b);
}

struct t_node* sortedMerge(struct t_node* a, struct t_node* b)
{
  struct t_node* result = NULL;
 
  /* Base cases */
  if (a == NULL)
     return(b);
  else if (b==NULL)
     return(a);
 
  /* Pick either a or b, and recur */
  if (a->weight <= b->weight)
  {
     result = a;
     result->next = sortedMerge(a->next, b);
  }
  else
  {
     result = b;
     result->next = sortedMerge(a, b->next);
  }
  return(result);
}

void frontBackSplit(struct t_node* source,
    struct t_node** front, struct t_node** back){
    struct t_node* next;
    struct t_node* current;
    if (source==NULL || source->next==NULL){
        /* length < 2 cases */
        *front = source;
        *back = NULL;
    }
    else{
        current = source;
        next = source->next;
        /* Advance 'next' two nodes, and advance 'current' one node */
        while (next != NULL){
            next = next->next;
            if (next != NULL){
                current = current->next;
                next = next->next;
            }
        }
        /* 'current' is before the midpoint in the list, so split it in two
        at that point. */
        *front = source;
        *back = current->next;
        current->next = NULL;
    }
}

void printQueue(struct pt_queue *queue){
    struct t_node* temp = queue->head;
    while(temp != NULL){
        printf("weight: %f time: %" PRIu64 , temp->weight, temp->time);
        temp = temp->next;
    }
    printf("\n");
}

//Test Case
int main(){
    struct pt_queue* head = createPt_queue();
    //create some nodes with different weights
    enqueue(head, createT_node(NULL));
    head->head->weight += .44563;
    enqueue(head, createT_node(NULL));
    head->head->weight += .2454;
    enqueue(head, createT_node(NULL));
    head->head->weight += .64363;
    enqueue(head, createT_node(NULL));
    head->head->weight += .964463;
    enqueue(head, createT_node(NULL));
    head->head->weight += .6869695;
    printf("length: %d\n", head->length);
    printQueue(head);
    mergeSort(&(head->head));
    printQueue(head);
    printf("length: %d\n", head->length);
    return 0;
}