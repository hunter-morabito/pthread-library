#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

void starttime();
void stoptime();

static int count = 0;
void namesd (int signum)
{
 count++;
}

void otherfunc(){
    stoptime();
    int i = 0;
    while(i < 1000000000){
        i++;
    };
    starttime();
}

void init(){
    struct sigaction sa;
    struct itimerval timer;

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &namesd;
    sigaction (SIGVTALRM, &sa, NULL);

    /* Configure the timer to expire after 250 msec... */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1;
    /* ... and every 250 msec after that. */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1;
    /* Start a virtual timer. It counts down whenever this process is
    executing. */
    starttime(&timer);
}

void stoptime(){
    struct itimerval timer;
    timer.it_interval.tv_usec = 1;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 1;
	timer.it_value.tv_sec = 0;
    setitimer (ITIMER_VIRTUAL, &timer, NULL);
}

void starttime(){
    struct itimerval timer;
	timer.it_interval.tv_usec = 1;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 1;
	timer.it_value.tv_sec = 0;
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
}

int main ()
{
    init();
    /* Do busy work. */
    while (1){
        printf("count: %d\n", count);
        otherfunc();
    };
}