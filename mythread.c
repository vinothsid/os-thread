#include "mythread.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/types.h>

int INIT_ONCE=1;

/*
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
*/
/*
* Enqueues thread in the run Q
*/
int enqueue(DNODE n) {
	return insertAtEnd(qHead,n);
} 

/*
* Creates and initialises the node to be joined in run Q
*/
DNODE createNode(){
	struct thread * t = (struct thread*)malloc(sizeof(struct thread));
	t->state = CREATED;
	t->threadId = -1;
	futex_init(&(t->selfLock),0);
	t->stackPtr = (char *)malloc(STACK_SIZE);
	t->joinQ = NULL;
	t->exitStatus = -1;
	
	DNODE newNode = getDllNode((void *)t);
	return newNode;	
}

/*
* Deques the node in the head of run Q
*/
int dequeue() {
	free(getMember(qHead,stackPtr));
	free(qHead->data);
	return dllDelete(qHead,&qHead);
}

/*
* Search for the threadId in the run Q and return the node
*/
DNODE search(int threadId) {
	return find(qHead,compare,(void *)&threadId);
}

/*Compare helper function for search()
*
*/
int compare(DNODE n,void *d) {

        int id = *(int *)d;
       // printf("Comparing %d and %d\n",getMember(n,threadId),id);
        if(getMember(n,threadId) == id)
                return 1;
        else
                return 0;

}

/* Helper function for searchRunnable
*
*/
int compareState(DNODE n,void *d) {

        int state = *(int *)d;
       // printf("Comparing %d and %d\n",getMember(n,threadId),id);
        if(getMember(n,state) == state)
                return 1;
        else
                return 0;

}

/*
* Search for first Runnable thread in the run Q 
*/

DNODE searchRunnable() {
	int state = RUNNABLE;
	return find(qHead,compareState,(void *)&state);
}

/*
* Print the run Q . Only for debugging . Contains prints so should not be used when multiple threads are running parallely.
*/
int printQ() {
	DNODE p = qHead;
	if(p==NULL) {
		printf("qHead is NULL\n");
		return -1;
	}
	
	printf("Queue elements\n");
	printf("==================\n");
	do {
		printf(" TID:%d ; STATE:%d ; Lock:%d ; exitStatus:%d \n",getMember(p,threadId) , getMember(p,state) , (getMember(p,selfLock)).count , getMember(p,exitStatus) );
		p = p->next;
	} while(p!=qHead);

	return 0;
}

/*
* Sets the state of all threads in Join Q RUNNABLE 
*/
int wake() {

	return 0;
}

/*
* Join the calling thread Node(pointed by head) to join Q of target thread 
*/
int joinQ(mythread_t thr ) {

}


