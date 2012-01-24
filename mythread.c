#include "mythread.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/types.h>

extern int INIT_ONCE=1;

int mythread_create(mythread *new_thread_ID,mythread_attr_t *attr,void *(*start_func)(void *), void* arg) {
	
	if(INIT_ONCE) {
		initThread();
		INIT_ONCE=0;
	}	
	
	DNODE newNode = createNode();
	futex_down(queueLock);
	enqueue(newNode);
	futex_up(queueLock);
	
	struct task t=(struct task*)malloc(sizeof(struct task));
	

}
