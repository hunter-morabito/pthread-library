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
#include <string.h>

ucontext_t maincontext;
/* quantum1 will have all weights at 0
*  quantum2 will have 1 - 5
*  quantum3 will have 6 - 10 except in 
*  cases where the threads have been waiting a long time */
pt_queue* wait_queue;
pt_queue* quantum1,* quantum2,* quantum3;
pt_queue* finishQueue; //may not need to implement
t_node* mainthread,* currentthread;
static int count = 0;

short cancel = 0; //when a thread should be canceled make this 1

void initThreadLib(){
	//initialize queues
	struct itimerval timer;
	struct sigaction sa;
	quantum1 = createPt_queue();
	if(quantum1 == NULL){
		exit(EXIT_FAILURE);
	}
	quantum2 = createPt_queue();
	if(quantum2 == NULL){
		exit(EXIT_FAILURE);
	}
	quantum3 = createPt_queue();
	if(quantum3 == NULL){
		exit(EXIT_FAILURE);
	}
	finishQueue = createPt_queue();
	if(finishQueue == NULL){
		exit(EXIT_FAILURE);
	}
	wait_queue = createPt_queue();
	if(wait_queue == NULL){
		exit(EXIT_FAILURE);
	}

	memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &scheduler;
	//initialize alarm values to zero
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;

	//create alarm signal
	if (sigaction(SIGVTALRM, &sa, NULL) == -1) {
        printf("error with: sigaction\n");
        exit(EXIT_FAILURE);
    }

	if ( getcontext(&maincontext)== -1) {
		printf("Error while getting context...exiting\n");
		exit(EXIT_FAILURE);
	}
	
	tcb* newtcb = createTCB(1);
	mainthread = createT_node(newtcb);
	currentthread = mainthread;
	
	starttime(1);
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
	pqueue->tail->next = newProcess;
	pqueue->tail = newProcess;
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
	//reset temp->next before deqeuing!
	temp->next = NULL;
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
	if (queue->head == NULL){
		printf("NULL");
		return;
	}
    while(temp != NULL){
        printf("pid: %d address: 0x%" PRIXPTR "->", temp->thread_block->tid, temp);
        temp = temp->next;
    }
    printf("\n");
}
//END PRIORITY QUEUE METHODS

void stoptime(){
	struct itimerval timer;
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void starttime(int us){
	struct itimerval timer;
	timer.it_interval.tv_usec = us;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = us;
	timer.it_value.tv_sec = 0;
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
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
			scheduler(1);
			return 1;
		}
	}
	return 0;
}

int remove_from_queue(my_pthread_t thread){
	t_node* prev;
	t_node* temp;
	pt_queue* queue = quantum1;
	for (temp = queue->head, prev = NULL; temp != NULL; prev = temp, temp = temp->next){
		if (temp->thread_block->tid == thread){
			queue->length--;
			if (prev == NULL){
				//remove the head
				queue->head = temp->next;
				return 1;
			}
			//skip over temp
			prev->next = temp->next;
			return 1;
		}
	}
	queue = quantum2;
	for (temp = queue->head, prev = NULL; temp != NULL; prev = temp, temp = temp->next){
		if (temp->thread_block->tid == thread){
			queue->length--;
			if (prev == NULL){
				//remove the head
				queue->head = temp->next;
				return 1;
			}
			//skip over temp
			prev->next = temp->next;
			return 1;
		}
	}
	queue = quantum3;
	for (temp = queue->head, prev = NULL; temp != NULL; prev = temp, temp = temp->next){
		if (temp->thread_block->tid == thread){
			if (prev == NULL){
				queue->length--;
				//remove the head
				queue->head = temp->next;
				return 1;
			}
			//skip over temp and enqueue in finish queue
			prev->next = temp->next;
			return 1;
		}
	}
	queue = finishQueue;
	for (temp = queue->head, prev = NULL; temp != NULL; prev = temp, temp = temp->next){
		if (temp->thread_block->tid == thread){
			return -1;
		}
	}
}

void scan(){
	uint64_t difference;
	//every 1000 us decrease prio by 1
	uint64_t mod;
	t_node* temp;
	pt_queue* queue;
	//go through quantum2 and quantum3
	queue = quantum2;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		difference = getTimeStamp() - temp->time;
		mod = difference / 1000000; //in milliseonds
		if (!(mod > temp->weight)) {
			temp->weight -= mod;
		}
		else {
			if (temp->weight >= 1)
				temp->weight -= 1;
		}
	}
	//perform merge sort on queue
	mergeSort(&queue->head);

	queue = quantum3;
	for (temp = queue->head; temp != NULL; temp = temp->next){
		difference = getTimeStamp() - temp->time;
		mod = difference / 1000000;
		if (!(mod > temp->weight)) {
			temp->weight -= mod;
		}
		else {
			if (temp->weight >= 1)
				temp->weight -= 1;
		}
	}
	mergeSort(&queue->head);
}

void scheduler(int signum){
	stoptime();
	//scan all queues to adjust priority
	scan();
	printf("cthread weight: %d\n", currentthread->weight);

	if (signum == 1){
		enqueue(wait_queue, currentthread);
	}
	else if (!cancel && currentthread != NULL){
		//enqueue back into quantum1 for now
			//means that signal called scheduler otherwise yield would call scheduler
			currentthread->weight += 5;
		if (currentthread->weight > 10)
			currentthread->weight = 10;
		if (currentthread->weight > 5){
			printf("enqueuing: 0x%" PRIXPTR " in quantum 3\n", currentthread);
			enqueue(quantum3, currentthread);
		}
		else if (currentthread->weight > 0){
			printf("enqueuing: 0x%" PRIXPTR "in quantum 2\n", currentthread);
			enqueue(quantum2, currentthread);
		}
		else{
			printf("enqueuing: 0x%" PRIXPTR "in quantum 1\n", currentthread);
			enqueue(quantum1, currentthread);
		}
	}
	else{
		cancel = 0;
	}

	//printf("count: %d\n", count);
	t_node* current;
	t_node* next_thread;
	//check first is something has very high priority
	//then check if something is in the run queue
	if (quantum3->head != NULL && quantum3->head->weight <= 1){
		//run this thread it has been waiting for a while
		current = currentthread;
		next_thread = dequeue(quantum3);
		printf("dequeuing from q3 first!: 0x%" PRIXPTR "\n", next_thread);
		currentthread = next_thread;
		currentthread->weight = 6;
		starttime(5);
		if (swapcontext(&(current->thread_block->context), 
			&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n");
		}
	}
	else if (quantum2->head != NULL && quantum2->head->weight <= 1){
		current = currentthread;
		next_thread = dequeue(quantum3);
		printf("dequeuing from q2 first!: 0x%" PRIXPTR "\n", next_thread);
		currentthread = next_thread;
		currentthread->weight = 1;
		starttime(5);
		if (swapcontext(&(current->thread_block->context), 
			&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n");
		}
	}
	else if (quantum1->head != NULL){
		//standard run queue
		current = currentthread;
		next_thread = dequeue(quantum1);
		printf("next_thread: 0x%" PRIXPTR "\n", next_thread);
		currentthread = next_thread;
		starttime(1);
		if (swapcontext(&(current->thread_block->context), 
			&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n");
		}
	}
	else if (quantum2->head != NULL){
		current = currentthread;
		next_thread = dequeue(quantum2);
		currentthread = next_thread;
		if (currentthread->weight < 1){
			currentthread->weight = 1;
		}
		starttime(2); //run for longer
		if (swapcontext(&(current->thread_block->context), 
		&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n"); 
		}
	}
	else if (quantum3->head != NULL){
		current = currentthread;
		next_thread = dequeue(quantum3);
		currentthread = next_thread;
		if (currentthread->weight < 6){
			currentthread->weight = 6;
		}
		starttime(5); //longest a thread can run for
		if (swapcontext(&(current->thread_block->context), 
		&(next_thread->thread_block->context)) == -1 ){
			printf("Error while swap context\n");
		}
	}
	else {
		//nothing to schedule :(
		releaseWait();
	}
	starttime(1);
}

//release everything fromt he waitqueue
void releaseWait(){
	t_node* temp;
	for (temp = wait_queue->head; temp != NULL; temp = temp->next){
		temp = dequeue(wait_queue);
		enqueue(quantum1, temp);
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
	starttime(1);
	scheduler(1);
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void* value_ptr){
	currentthread->thread_block->returnvalue = value_ptr;
	enqueue(finishQueue, currentthread);
	pthread_cancel(currentthread->thread_block->tid);
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void** value_ptr){
	
	short threadEnded = 0;
	t_node* cur;

	while(threadEnded == 0){
		if(finishQueue->head!=NULL){
			cur = finishQueue->head;

			while(cur!=NULL){
				if(cur->thread_block->tid == thread)
					threadEnded = 1;
				cur=cur->next;
			}
		}

		if(threadEnded == 0)
			my_pthread_yield();
	}
	return 0;
};

/* initialize the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t* mutex, const pthread_mutexattr_t* mutexattr){
	if(mutex != NULL)
		return(-1); // failed, mutex is already initialized

	stoptime();
	mutex = (my_pthread_mutex_t*)malloc(sizeof(my_pthread_mutex_t));
	if(mutex == NULL){
		printf("memory allocation error");
		starttime(10);
		return(-1);
	}

	mutex->locked = 0;
	mutex->holder = 0;
	starttime(1);
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t* mutex){
	if(mutex == NULL)
		return(-1); // failed, mutex is not initialized

	stoptime();
	while(mutex->locked != 0){
		my_pthread_yield();
	}
	
	mutex->locked = 1;
	mutex->holder = currentthread->thread_block->tid; // must be assigned to my_pthread_t of current thread
	starttime(1);
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t* mutex){
	
	stoptime();
	if(mutex == NULL || mutex->holder == currentthread->thread_block->tid){ // must compare to my_pthread_t of current thread
		starttime(1);
		return(-1); // failed, mutex is not initialized
	}

	mutex->locked=0;
	mutex->holder=0;
	releaseWait();
	starttime(1);

	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t* mutex){
	
	stoptime();
	if(mutex==NULL || mutex->locked!=0){
		starttime(1);
		return(-1); // failed, mutex is not initialized or is in use
	}

	free(mutex);
	starttime(1);
	return 0;
};

void* testfuc2(void* a){
	int i = 0;
	while (i < 10000000){
		i++;
	};
}

//TEST SECTION
void* testfuc(void* a){
	while (count < 10000000){
		count++;
	};
}

my_pthread_t t1 = 2;
my_pthread_t t2 = 3;
//test the code
int main(){
	initThreadLib();
	printf("mainthread: 0x%" PRIXPTR " id: %d\n", mainthread, mainthread->thread_block->tid);
	my_pthread_create(&t1,NULL,&testfuc, (void *) 1);
	my_pthread_create(&t2,NULL,&testfuc2, (void *) 1);
	printQueue(quantum1);
	while(1){
		if (mainthread != currentthread){
			printf("mainthread: 0x%" PRIXPTR "\n", currentthread);
		}
	}; //busy work
	return 0;
}

