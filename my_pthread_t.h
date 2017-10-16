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

typedef uint my_pthread_t;

typedef struct threadControlBlock {
	/* add something here */
	my_pthread_t tid;
	//timer value
	struct itimerval *it;
	//signal actions
	struct sigaction *action, *oaction;
	ucontext_t *context;
}tcb; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
} my_pthread_mutex_t;

/* define your data structures here: */

//start queue functions
//struct for thread nodes
typedef struct t_node{
	//pointer to threadblock
	tcb *thread_block;
	//pointer to next threadblock in queue
	struct t_node *next;
}t_node;

//queue object which holds the thread queue nodes
//useful when deadling with programs with multiple queues
typedef struct t_queue{
	//define head and tail nodes of queue
	struct t_node *head, *tail;
	//hold the number of threads in queue
	unsigned int numOfThreads;
}t_queue;

//define function for creating thread node
struct t_node * createT_node(tcb * threadblock);
// define function for creating thread queue
struct t_queue * createT_queue();
//define function for inserting t_node into queue
void enQueue(t_queue * queue, tcb * threadBlock);
//define function for retrieving node from given queue
struct t_node * deQueue(t_queue * queue);
//end queue functions

// Feel free to add your own auxiliary data structures


/* Function Declarations: */

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
