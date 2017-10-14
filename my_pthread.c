// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Hunter Morabito
// username of iLab: hjm67
// iLab Server: utility.cs.rutgers.edu

#include "my_pthread_t.h"
#include <stdio.h>

void initThreadLib(){
	struct t_queue *runQ = createT_queue();
	struct t_queue *waitQ = createT_queue();
}

//start queue funtions
//creates new thread node and returns it
struct t_node *createT_node(tcb *threadblock){
	//create new threadNode
	struct t_node *newNode = (struct t_node*)malloc(sizeof(struct t_node));
	//set attributes of t_node
	newNode->thread_block = threadblock;
	//have the pointer point to the thread location
	newNode->next = NULL;
	//return new node
	return newNode;
}

//create a thread queue object and return ir
struct t_queue *createT_queue(){
	//allocate memory
	struct t_queue *queue = (struct t_queue*)malloc(sizeof(struct t_queue));
	//init head and tail
	queue->head = queue->tail = NULL;
	//init numofthreads
	queue->numOfThreads = 0;
	return queue;
}

//enqueue function takes in the queue you wish to add to and the data that the node will hold
void enQueue(struct t_queue *queue, tcb *threadblock){
	//create node
	struct t_node *temp = createT_node(threadblock);
	//empty check
	if(queue->numOfThreads == 0){
		queue->head = queue->tail = temp;
		return;
	}
	
	//add node to end of queue
	queue->tail->next = temp;
	//update tail to new node position
	queue->tail = temp;
	//increment number of threads in the queue
	queue->numOfThreads++;
}

//dequeue funtion returns the front of the given queue
struct t_node *deQueue(struct t_queue *queue){
	if(queue->numOfThreads == 0){
		return NULL;
	}
	//hold onto previous head
	struct t_node *temp = queue->head;
	//update the head node to the next node
	queue->head = queue->head->next;
	//decrement the number of threadas you have in queue
	queue->numOfThreads--;
	//if front is now NULL, also update rear to NULL
	if(queue->numOfThreads == 0)
		queue->tail = NULL;
	//return thread
	return temp;
}//end queue functions


/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	/*if(getcontext(&thread -> ucs) == -1){
		fprintf(stderr,"error: couldn't get context");
		exit(1);	
	}*/
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	return 0;
};

