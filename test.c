#include "my_pthread_t.h"
#include <stdio.h>

int main(int argc, char ** argv){
	t_queue *runqueue = createT_queue();
	tcb * newThread = (tcb*)malloc(sizeof(tcb));
	enQueue(runqueue, newThread);
	enQueue(runqueue,newThread);
	deQueue(runqueue);
}
