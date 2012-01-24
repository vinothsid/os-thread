#include "mythread.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/types.h>
#include <signal.h>

int INIT_ONCE=1;

int scheduler() {
	while(getppid()) {
		futex_down(&queueLock);
		DNODE toBeRun=NULL;//=searchRunnable();	
		futex_up(&queueLock);
		futex_up(&getMember(toBeRun,selfLock));
		futex_down(&schedulerLock);
	}
	return 0;
}

int mythread_create(mythread_t *new_thread_ID,mythread_attr_t *attr,void *(*start_func)(void *), void* arg) {
	
	if(INIT_ONCE) {
		initThread();
		INIT_ONCE=0;
	}	
	
	DNODE newNode = createNode();
	futex_down(&queueLock);
	enqueue(newNode);
	futex_up(&queueLock);
	
	//creating task struct below
	struct task t;
	t.func=start_func;
	t.arg=(void *)arg;
	t.qPos=newNode;
	//end
	
	*new_thread_ID=clone(&mythread_wrapper,getMember(newNode,stackPtr)+(8*1024),SIGCHLD|CLONE_FILES|CLONE_FS|CLONE_VM,(void *)(&t));
	
	return 0;
}


int mythread_wrapper(void *t) {
	struct task* tWrapper=(struct task*)t;
	getMember(tWrapper->qPos,threadId)=mythread_self();
	getMember(tWrapper->qPos,state)=RUNNABLE;
	futex_down(&getMember(tWrapper->qPos,selfLock));
	(tWrapper->func)(tWrapper->arg);
	wake(); //works on the assumption that qHead points to self
	futex_down(&queueLock);
	dequeue();
	qHead=qHead->next;
	futex_up(&queueLock);
	futex_up(&schedulerLock);
	return 0;
}
