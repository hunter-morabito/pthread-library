// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Hunter Morabito
// username of iLab: hjm67
// iLab Server: utility.cs.rutgers.edu
#include "my_pthread_t.h"
#include <stdio.h>


short dontinterrupt = 0; //when a thread should not be interrupted, make this =1; in sighandler, if this = 1, do not change context


void initThreadLib(){
}


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

	if(mutex!=NULL)
		return(-1); // failed, mutex is already initialized

	mutex->locked = 0;
	mutex->holder = 0;
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {

	if(mutex==NULL)
		return(-1); // failed, mutex is not initialized

	while(mutex->locked!=0)
		my_pthread_yield();
	
	dontinterrupt = 1;
	mutex->locked=1;
	mutex->holder=; // must be assigned to my_pthread_t of current thread
	dontinterrupt = 0;
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {

	if(mutex==NULL || mutex->holder == ) // must compare to my_pthread_t of current thread
		return(-1); // failed, mutex is not initialized

	dontinterrupt = 1;
	mutex->locked=0;
	mutex->holder=0;
	dontinterrupt = 0;

	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	
	if(mutex==NULL || mutex->locked!=0)
		return(-1); // failed, mutex is not initialized or is in use

	free(mutex);
	return 0;
};

