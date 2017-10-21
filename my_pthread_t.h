// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server: 
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include<stdint.h>

typedef uint my_pthread_t; // should not be 0

typedef struct threadControlBlock {
	void* returnvalue;
	my_pthread_t tid;
	//timer value	
	struct itimerval* it;
	//signal actions
	struct sigaction* action,* oaction;
	ucontext_t context; 
}tcb; 

typedef struct t_node{
	//weight and time values for priority queue and scan function
	uint64_t weight;
	uint64_t time;
	tcb* thread_block;
	struct t_node* next;
}t_node;

typedef struct pt_queue{  
    struct t_node* head,* tail;
    int length;
}pt_queue;

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	
	short locked; // 0 when unlocked, 1 when locked
	my_pthread_t holder; // if unlocked value is 0; if locked value is thread that locked the mutex

} my_pthread_mutex_t;

/* define your data structures here: */

//merge sort functions
void mergeSort(t_node** pqueue);
struct t_node* sortedMerge(struct t_node* a, struct t_node* b);
void frontBackSplit(struct t_node* source, struct t_node** front, struct t_node** back);
void printQueue(struct pt_queue *queue);

//goes through all pq and adjusts weight based on runtime
void scan();

// define function for creating thread queue and node
struct pt_queue* createPt_queue();
struct t_node* createT_node();
struct threadControlBlock* createTCB(my_pthread_t pid);

//define function for inserting t_node into queue
void enqueue(struct pt_queue* queue, struct t_node* newProcess);

//define function for retrieving node from given queue
struct t_node* dequeue(struct pt_queue* queue);


//Other
void scheduler(int signum);
uint64_t getTimeStamp();
void starttime(int us);
void stoptime();
void initThreadLib();
void runThread(void* (*func)(void*), void* arg);
int remove_from_queue(my_pthread_t thread);
void releaseWait();

/* Function Declarations: */
void interrupt_handler();
/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initialize the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#ifdef USE_MY_PTHREAD
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define pthread_create my_pthread_create
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#endif

#endif