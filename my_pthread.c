// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Hunter Morabito
// username of iLab: hjm67
// iLab Server: utility.cs.rutgers.edu
#include "my_pthread_t.h"
#include <stdio.h>
#include <sys/ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>

ucontext_t maincontext;
/* quantum1 will have all weights at 0
*  quantum2 will have .5 - .11
*  fcfs will have 1 - .49 except in 
*  cases where the threads have been waiting a long time */
pt_queue* quantum1,* quantum2,* fcfs;
pt_queue* finishQueue; //may not need to implement
t_node* mainthread,* currentthread;
struct itimerval timer;
struct sigaction* action;

short cancel = 0; //when a thread should be canceled make this 1
short dontinterrupt = 0; //when a thread should not be interrupted, make this =1; in sighandler, if this = 1, do not change context

void initThreadLib(){
	//initialize queues
	quantum1 = createPt_queue();
	if(quantum1 == NULL){
		exit(EXIT_FAILURE);
	}
	quantum2 = createPt_queue();
	if(quantum2 == NULL){
		exit(EXIT_FAILURE);
	}
	fcfs = createPt_queue();
	if(fcfs == NULL){
		exit(EXIT_FAILURE);
	}
	finishQueue = createPt_queue();
	if(finishQueue == NULL){
		exit(EXIT_FAILURE);
	}
	//initialize alarm values to zero
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;

	//create alarm signal
	signal(SIGVTALRM, scheduler);

	if ( getcontext(&maincontext)== -1) {
		printf("Error while getting context...exiting\n");
		exit(EXIT_FAILURE);
	}
	
	tcb* newtcb = createTCB(-1);
	mainthread - createT_node(newtcb);
	currentthread = mainthread;
	starttime(10);
}

/*priority queue methods*/
uint64_t getTimeStamp(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

struct t_node* createT_node(tcb* threadblock){
	//create new threadNode
	struct t_node* newNode = (struct t_node*)malloc(sizeof(struct t_node));
	if (newNode == NULL){
		printf("memory allocation error");
		return NULL;
	}
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
	if (queue == NULL){
		printf("memory allocation error");
		return NULL;
	}
	//init head and tail
    queue->head = NULL;
    queue->tail = NULL;
	//init length
	queue->length = 0;
	return queue;
}

struct threadControlBlock* createTCB(my_pthread_t pid){
	tcb * newTCB = (tcb *)malloc(sizeof(tcb));
	if (newTCB == NULL){
		printf("memory allocation error");
		return NULL;
	}
	if ( getcontext(&(newTCB->context)) == -1) {
		printf("Error while getting context...exiting\n");
		exit(EXIT_FAILURE);
	}
	newTCB->tid = pid;
	newTCB->context.uc_link = &maincontext;
	newTCB->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
	newTCB->context.uc_stack.ss_size = SIGSTKSZ;
	return newTCB;
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
void mergeSort(struct t_node** pqueue){
  struct t_node* head = *pqueue;
  struct t_node* a;
  struct t_node* b;
 
  if ((head == NULL) || (head->next == NULL)){
    return;
  }
 
  frontBackSplit(head, &a, &b); 
 
  mergeSort(&a);
  mergeSort(&b);
 
  *pqueue = sortedMerge(a, b);
}

struct t_node* sortedMerge(struct t_node* a, struct t_node* b){
  struct t_node* result = NULL;
 
  if (a == NULL)
     return(b);
  else if (b==NULL)
     return(a);
 
  if (a->weight <= b->weight){
     result = a;
     result->next = sortedMerge(a->next, b);
  }
  else{
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
        *front = source;
        *back = NULL;
    }
    else{
        current = source;
        next = source->next;
        while (next != NULL){
            next = next->next;
            if (next != NULL){
                current = current->next;
                next = next->next;
            }
        }
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
//END PRIORITY QUEUE METHODS

void stoptime(){
	setitimer(ITIMER_VIRTUAL, 0, 0);
}

void starttime(int us){
	struct itimerval old, new;
	new.it_interval.tv_usec = 0;
	new.it_interval.tv_sec = 0;
	new.it_value.tv_usec = 0;
	new.it_value.tv_sec = us;
	if (setitimer (ITIMER_REAL, &new, 0) < 0)
	  return;
}

void runThread(void* (*func)(void*), void* arg){
	my_pthread_exit(func(arg));
	return;
}

int pthread_cancel(my_pthread_t thread){
	//go through the ready queues and remove the thread
	t_node* temp;
	if (remove_from_queue(thread) != 0){
		//check if it is currently running thread
		if (thread == currentthread->thread_block->tid){
			cancel = 1;
			scheduler();
			return 1;
		}
	}
	else{

	}

}

int remove_from_queue(my_pthread_t thread){
	t_node* prev = NULL;
	t_node* temp;
	pt_queue* queue = quantum1;
	if (queue->length == 0){
		return 0;
	}

	for (temp = queue->head; temp != NULL; prev = temp, temp = temp->next){
		if (temp->thread_block->tid == thread){
			queue->length--;
			if (prev == NULL){
				//remove the head
				queue->head = temp->next;
				enqueue(finishQueue, temp);
				return 1;
			}
			//skip over temp
			prev->next = temp->next;
			enqueue(finishQueue, temp);
			return 1;
		}
	}
	queue = quantum2;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		if (temp->thread_block->tid == thread){
			queue->length--;
			if (prev == NULL){
				//remove the head
				queue->head = temp->next;
				enqueue(finishQueue, temp);
				return 1;
			}
			//skip over temp
			prev->next = temp->next;
			enqueue(finishQueue, temp);
			return 1;
		}
	}
	queue = fcfs;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		if (temp->thread_block->tid == thread){
			if (prev == NULL){
				queue->length--;
				//remove the head
				queue->head = temp->next;
				enqueue(finishQueue, temp);
				return 1;
			}
			//skip over temp and enqueue in finish queue
			prev->next = temp->next;
			enqueue(finishQueue, temp);
			return 1;
		}
	}
	queue = finishQueue;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		if (temp->thread_block->tid == thread){
			if (prev == NULL){
				//if it is found in finish queue it has already been removed at some other point
				return -1;
			}
		}
	}
}

//returns 1 if thread exists in one of the ready queues, 0 if its in the finish queue and -1 if it does not exist
/*
int searchThread(my_pthread_t pid){
	pt_queue* queue = quantum1;
	node_t* temp;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		if (temp->tid == pid){
			return 1;
		}
	}
	queue = quantum2;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		if (temp->tid == pid){
			return 1;
		}
	}
	queue = fcfs;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		if (temp->tid == pid){
			return 1;
		}
	}
	queue = finishQueue;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		if (temp->tid == pid){
			return 0;
		}
	}
	return -1;
}*/

void scheduler(){
	stoptime();
	//scan all queues to adjust priority
	//scan();
	if (dontinterrupt == 1){
		//do not switch contexts
	}

	if (!cancel && currentthread != NULL){
		//enqueue back into quantum1 for now
		enqueue(quantum1, currentthread);
	}

	t_node* current;
	t_node* next_thread;
	//check first is something has very high priority
	//then check if something is in the run queue
	if (fcfs->head->weight <= .1){
		//run this thread it has been waiting for a while
	}
	else if (quantum2->head->weight <=.1){

	}

	if (quantum1->head != NULL){
		//standard run queue
		current = currentthread;
		next_thread = dequeue(quantum1);
		currentthread = next_thread;
		starttime(10);
		if (swapcontext(&(current->thread_block->context), 
			&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n"); /*calling the next thread*/
		}
	}
	else if (quantum2->head != NULL){
		current = currentthread;
		next_thread = dequeue(quantum1);
		currentthread = next_thread;
		starttime(20); //run for longer
		if (swapcontext(&(current->thread_block->context), 
		&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n"); /*calling the next thread*/
		}
	}
	else if (fcfs->head != NULL){
		current = currentthread;
		next_thread = dequeue(quantum1);
		currentthread = next_thread;
		starttime(50); //longest a thread can run for
		if (swapcontext(&(current->thread_block->context), 
		&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n"); /*calling the next thread*/
		}
	}
	else {
		//nothing to schedule :(
	}
}

/* create a new thread */
int my_pthread_create(my_pthread_t* thread, pthread_attr_t* attr, void *(*function)(void*), void * arg){
	tcb* newThread = createTCB(*thread);
	if(getcontext(&(newThread->context)) == -1){
		fprintf(stderr,"error: couldn't get context");
		exit(EXIT_FAILURE);	
	}
	//we get a pointer to a uint so we must deference it
	if (newThread == NULL){
		printf("fatal memory allocation error");
		exit(EXIT_FAILURE);
	}

	makecontext(&(newThread->context), (void (*) ()) runThread, 2, function, arg);
	t_node* newNode = createT_node(newThread);
	enqueue(quantum1, newNode);

	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield(){
	stoptime();
	starttime(10);
	scheduler();
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void* value_ptr){
	currentthread->thread_block->returnvalue = value_ptr;
	enqueue(finishQueue, currentthread);
	printQueue(finishQueue);
	pthread_cancel(currentthread->thread_block->tid);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void** value_ptr){
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t* mutex, const pthread_mutexattr_t* mutexattr){
	if(mutex != NULL)
		return(-1); // failed, mutex is already initialized

	mutex->locked = 0;
	mutex->holder = 0;
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t* mutex){
	if(mutex == NULL)
		return(-1); // failed, mutex is not initialized

	while(mutex->locked != 0)
		my_pthread_yield();
	
	stoptime();
	mutex->locked = 1;
	mutex->holder = currentthread->thread_block->tid; // must be assigned to my_pthread_t of current thread
	starttime(10);
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t* mutex){
	if(mutex == NULL || mutex->holder == currentthread->thread_block->tid) // must compare to my_pthread_t of current thread
		return(-1); // failed, mutex is not initialized

	stoptime();
	mutex->locked=0;
	mutex->holder=0;
	starttime(10);

	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t* mutex){
	if(mutex==NULL || mutex->locked!=0)
		return(-1); // failed, mutex is not initialized or is in use

	free(mutex);
	return 0;
};

//TEST SECTION
void* testfuc(void* a){
	printf("doing things");
}

my_pthread_t t1;
//test the code
int main(){
	initThreadLib();
	my_pthread_create(&t1,NULL,&testfuc, (void *) 1);
	return 0;
}

