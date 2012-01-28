#include "mythread.h"
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <sys/types.h>
#include <signal.h>

int ALREADY_UP=0;
int INIT_ONCE=1;
DNODE qHead = NULL;
DNODE idleNode = NULL;
DNODE mainNode = NULL;
int ZERO_THREADS=0;
/*
*declarations for testing only
*/
mythread_key_t testkey1, testkey2, testkey3;


/*End of test declarations
*/
char* itoa(int val, int base){
        static char buf[32] = {0};
        int i = 30;
        for(; val && i ; --i, val /= base)
                buf[i] = "0123456789abcdef"[val % base];
        return &buf[i+1];
}
/*
mythread_join() 
1. queueLock
2. searchRunnable
3. if qHead==idleNode set ALREADY_UP=1
4. search for tid in queue
5. Add self node ptr to this tid nodes' join Q
6. set self state to WAIT
7. wake up the RUNNABLE from step 2
8. release queueLock i.e. futex_down(&queueLock)
9. lock self Lock and wait to be awakened
*/

int mythread_join( mythread_t target_thread , void **status ) {
//	printf("self id : %d , head id : %d\n",mythread_self(),getMember(qHead,threadId) );
	
	if( mythread_self() == getMember(qHead,threadId) ) {
//		assert()
		print("mythread_join() : in if\n");
		futex_down(&queueLock);
		if ( joinQ(target_thread) == -1 ) {
			futex_up(&queueLock);
			return 0;
		}
		DNODE tmp = qHead;

		qHead = qHead->next;
		qHead = searchRunnable();
		//if search runnable returns idleNod
		//below code ensures that idleNode is 
		//not woken twice;;;	
		if (qHead == idleNode )
			ALREADY_UP = 1;

	//	DNODE tThread = search(target_thread);
		getMember(tmp,state) = WAIT;	
		futex_up(&(getMember(qHead,selfLock)));

		futex_up(&queueLock);
		futex_down(&(getMember(tmp,selfLock)));
	} else {
		//below code handles the scenario when main() calls
		//join;;
		futex_down(&queueLock);
		print("mythread_join() : in else\n");
		if ( joinQ(target_thread) == -1 ) {
			//print("could not join queue\n");
			futex_up(&queueLock);
                        return 0;

		}
		futex_up(&queueLock);
		futex_down(&(getMember(mainNode,selfLock)));
	}
	return 0;
	
}

int mythread_yield() {
	futex_down(&queueLock);
	DNODE temp=qHead;
	if(temp->next->next == temp) {
		//futex_up(&(getMember(idleNode,selfLock)));
		print("here in if\n");
		futex_up(&queueLock);
	} else {
		print("here in else\n");
		//DNODE temp=qHead;
		qHead=qHead->next;
		qHead=searchRunnable();
		if(qHead==idleNode) {
			ALREADY_UP=1;
			print("yielded to idleNode\n");
		} 
		futex_up(&(getMember(qHead,selfLock)));
		futex_up(&queueLock);
		futex_down(&(getMember(temp,selfLock)));		
	}
	print("unblocked after yield\n");
	return 0;
}

void mythread_exit(void* retVal) {
	if(( mythread_self() != getMember(qHead,threadId))) {
		while(ZERO_THREADS!=0) {}
		exit(0);
	} else {
		futex_down(&queueLock);
			//*((int*)retVal)=0;
			wake();
			//dequeue();
			getMember(qHead,state) = DEAD;
			ZERO_THREADS--;
			qHead  = searchRunnable();	
			if(qHead==idleNode) {
				ALREADY_UP=1;
				print("yielded to idleNode\n");
			}	 
		futex_up(&(getMember(qHead,selfLock)));
		futex_up(&queueLock);
		futex_down(&exitLock);
	}
	//futex_down(&(getMember(temp,selfLock)));
}

int idleFunc() {
        DNODE toBeRun;
	while(getppid()) {
		print("In idle thread\n");
		futex_down(&(getMember(idleNode,selfLock)));
		print("In idle thread after idleNode lock release\n");
		futex_down(&queueLock);
		print("In idle thread after lock acquire\n");
		//qHead=qHead->next;	
		ALREADY_UP=0;
        	qHead=searchRunnable();
	        futex_up(&getMember(qHead,selfLock));
	
                futex_up(&queueLock);
	
		if(qHead==NULL)
			print("QHEAD is null\n");
	}
	return 0;
}

int initThread() {
	futex_up(&queueLock);
	idleNode=createNode();
	getMember(idleNode,state) = RUNNABLE;
	qHead=idleNode;
	initKeyArr();	
	mainNode=createNode();
	getMember(mainNode,threadId) = 0;
	futex_init(&mainLock,0);	
	futex_init(&exitLock,0);	
	getMember(idleNode,threadId) = clone(idleFunc,(getMember(idleNode,stackPtr))+STACK_SIZE,SIGCHLD|CLONE_VM|CLONE_FILES | CLONE_FS | CLONE_SIGHAND | CLONE_IO | CLONE_CHILD_CLEARTID,0);
	return 0;	
}

static pid_t gettid(void) {
	    return (pid_t) syscall(SYS_gettid);
}

mythread_t mythread_self() {
	return (mythread_t)gettid();
}



int mythread_create(mythread_t *new_thread_ID,mythread_attr_t *attr,void *(*start_func)(void *), void* arg) {
	
	if(INIT_ONCE) {
		initThread();
		INIT_ONCE=0;
		//while(qHead==NULL) {}
	}	
	
	DNODE newNode = createNode();
	//print("Before queueLock\n");
	futex_down(&queueLock);
	enqueue(newNode);

	print("After queueLock acquire in create_thread\n");	
	//creating task struct below
	struct task *t = (struct task *)malloc(sizeof(struct task));
	t->func=start_func;
	t->arg=(void *)arg;
	t->qPos=newNode;
	//end
	//print("After initialising task\n");
	
	*new_thread_ID = clone(&mythread_wrapper,getMember(newNode,stackPtr)+STACK_SIZE,SIGCHLD|CLONE_VM |CLONE_FILES | CLONE_FS | CLONE_SIGHAND | CLONE_IO|CLONE_CHILD_CLEARTID ,(void *)(t));

	
	//futex_up(&queueLock);
	//need to lock main here so mythread_join can function properly 
	//this lock will be released by the wrapper after fields necessary
	//to perform join are populated inside wrapper;;;
	//futex_down(&(getMember(mainNode,selfLock)));
	futex_down(&mainLock);
	return 0;
}

int mythread_wrapper(void *t) {
	struct task* tWrapper=(struct task*)t;
	getMember(tWrapper->qPos,threadId)=mythread_self();
	getMember(tWrapper->qPos,state)=RUNNABLE;
	//futex_up(&(getMember(mainNode,selfLock)));
	print("releasing mainLock\n");
	futex_up(&mainLock);
	//futex_down(&queueLock);
	if((qHead == idleNode) &&(!ALREADY_UP)) {
		ALREADY_UP=1;
		futex_up(&(getMember(idleNode,selfLock)));
		print("released idleNode lock\n");
	}
	futex_up(&queueLock);	
		
	print("before locking self\n");
	futex_down(&(getMember(tWrapper->qPos,selfLock)));
	print("after releasing self lock\n");

	//print("Scheduler released the selfLock\n");
	(tWrapper->func)(tWrapper->arg);

	futex_down(&queueLock);
        wake(); //works on the assumption that qHead points to self
                //wake changes state from WAIT to RUNNABLE
                //doesn't change the lock values and hence doesn't cause
                //threads to run;;;;
	//dequeue() will make qHead point to the next node;
	dequeue();
	//below code ensures that the idleNode is lock is released so that 
	//it can schedule the next thread to run when the only node remaining is
	//not the idleNode;;;
	//when only idleNode remains no lock is released, instead wait for a new
	//thread to be created to release this lock;;
	//As a corollary, it is assumed that ALREADY_UP will be set to zero by the
	//idleNode;; 
	if(qHead->next!=qHead) {
		ALREADY_UP=1;
		futex_up(&getMember(idleNode,selfLock));
		print("idleNode lock released in wrapper if condition\n");
	}
	//print("node dequeued\n");
	//qHead=qHead->next; //done in dequeu;
	futex_up(&queueLock);
	//futex_up(&schedulerLock);

	return 0;
}
/*
* Enqueues thread in the run Q
*/
int enqueue(DNODE n) {
	ZERO_THREADS++;//counter used to do an exit from main;
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
	memset(t->data_block,0,MAX_KEYS*sizeof(void*));
	DNODE newNode = getDllNode((void *)t);
	return newNode;	
}

/*
* Deques the node in the head of run Q
*/
int dequeue() {
	ZERO_THREADS--;
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
	if(qHead==idleNode) {
		print("idleNode found in searchRunnable\n");
		return find(qHead->next,compareState,(void *)&state) ;
	}
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

	LNODE p = getMember(qHead,joinQ);
	while( p  != NULL) {

		getMember(p->threadNode,state) = RUNNABLE;
		if(getMember(p->threadNode,threadId) == 0 ) { //it is  main thread 
			print("Releasing main lock\n");
			futex_up(&(getMember(mainNode,selfLock)));
			
		}
		p = p->next;

	}
 
	return 0;
}

/*
* Join the calling thread Node(pointed by head) to join Q of target thread .queueLock must be acquired before calling this function
*/
int joinQ(mythread_t threadId ) {
	DNODE targetThread = search( threadId );

	if( targetThread == NULL ) 
		return -1;

	if( getpid() == getMember(qHead,threadId) ) {
	        LNODE newNode = (LNODE) malloc(sizeof( struct joinQNode));
	        newNode->threadNode =  qHead ;
        	newNode->next = getMember(targetThread,joinQ);

	        getMember(targetThread,joinQ) = newNode;

	} else {
	//	LNODE jQueue = getMember(qHead,joinQ);

		LNODE newNode = (LNODE) malloc(sizeof( struct joinQNode));
		newNode->threadNode =  mainNode ;
		newNode->next = getMember(targetThread,joinQ);

		getMember(targetThread,joinQ) = newNode;

	}
	return 0;
/*
	if (jQueue == NULL) {
		getMember(qHead,joinQ) = (LNODE) malloc(sizeof( struct joinQNode));
		
	}
*/	
		
}
/*
*Block containing all function to allocate,set keys and
*thread specific data
*/
void initKeyArr(void) {
        int i=0;
        LOWEST_VALID_KEY=0;
        for(i=0;i<MAX_KEYS;i++) {
                key_arr[i].sane=0;
        }
}

int mythread_key_create(mythread_key_t *key, void (*destructor)(void*)) {
                int i;
                for(i=0;i<MAX_KEYS;i++) {
                        if(key_arr[i].sane==0) {
                                LOWEST_VALID_KEY=i;
                                *key=LOWEST_VALID_KEY;
                                key_arr[LOWEST_VALID_KEY].sane=1;
                                //printf("keyVal is %d \n",*key);
                                return 1;
                        }
                }
}


int mythread_setspecific(mythread_key_t key, const void *value) {
        if(key_arr[key].sane == 1) {
                getMember(qHead,data_block[key])=value;
                return 1;
        } else {
                return 0;
        }
}

void *mythread_getspecific(mythread_key_t key) {
	if(key_arr[key].sane==1)  {
		if(getMember(qHead,data_block[key])==NULL)
			print("key data has a null value \n");
		return getMember(qHead,data_block[key]);
	}
}	

int mythread_key_delete(mythread_key_t key) {
	if(key < MAX_KEYS) {
		key_arr[key].sane=0;//invalidates the key
		futex_down(&queueLock);
			DNODE temp = qHead;
			do {
				getMember(temp,data_block[key])=NULL;
				temp=temp->next;
			} while(temp!=qHead);
		futex_up(&queueLock);
		return 1;
	} else {
		return 0;
	}
	
}
void* sayHello7788() {
	print("I say hello7788\n");
	//exit(1);
}
void* sayHello() {
//	while(1) {
		printOut("I say hello\n");
		mythread_key_create(&testkey1,NULL);
		char* a="sayHello1:key";
		mythread_setspecific(testkey1,(void *)a);
		mythread_t tidx;
		mythread_yield();
		printOut("====test value 1=====\n");
		printOut((char *)mythread_getspecific(testkey1));
		printOut("\n=====**********======\n");
		printOut("I say hello\n");
//	}
	//mythread_create(&tidx,NULL,sayHello7788,NULL);
	//mythread_yield();
	//printOut("IN the grandest parent sayHello\n");
	//sleep(1);
	//exit(1);
}

void* sayHello6() {
	printOut("I say hello66\n");
	char* a="threadSpecific6";
	mythread_setspecific(testkey1,(void *)a);
	mythread_t tidx;
	mythread_yield();
	printOut("====test value 4=====\n");
	if(mythread_getspecific(testkey1)!=NULL)
		printOut((char *)mythread_getspecific(testkey1));
	else
		printOut("value is NULL\n");
	printOut("\n=====**********======\n");
	//exit(1);
}
void* sayHello3() {
//	while(1) {
		printOut("I say hello3\n");
		mythread_yield();
		char* a="threadSpecific3";
		mythread_setspecific(testkey1,(void *)a);
		mythread_t tidx;
		mythread_yield();
		printOut("====test value 3=====\n");
		printOut((char *)mythread_getspecific(testkey1));
		printOut("\n=====**********======\n");
		mythread_key_delete(testkey1);
		mythread_t tid6;
		mythread_create(&tid6,NULL,sayHello6,NULL);
		//mythread_join(tid6,NULL);
		printOut("I say hello3\n");
//	}
}
void* sayHello2() {
//	while(1) {
		mythread_t tid3;
		printOut("I say hello2\n");
		int *a="threadSpecific:2";
		mythread_setspecific(testkey1,(void *)a);
		mythread_t tidx;
		mythread_exit(NULL);
		mythread_yield();
		printOut("====test value 2=====\n");
		printOut((char *)mythread_getspecific(testkey1));
		printOut("\n=====**********======\n");
		mythread_yield();
		mythread_create(&tid3,NULL,sayHello3,NULL);
		//mythread_join(tid3,NULL);
		printOut("I say hello2\n");
//	}
}





int main() {
/*
	initKeyArr();
	futex_up(&queueLock);
	mythread_key_t testkey, testkey2;
	DNODE node1 = createNode();
	getMember(node1,threadId) = 1;
	qHead = node1;
	getMember(node1,state) = RUNNABLE;
	mythread_key_create(&testkey,NULL);
	printf("The value of key is %d\n",testkey);
	int* testdata=(int*)malloc(sizeof(int));
	*testdata=54;
	mythread_setspecific(testkey,(void *)testdata);
	printf("The thread specific data is %d\n",*((int*)(mythread_getspecific(testkey))));
	DNODE node2 = createNode();
	getMember(node2,threadId) = 2;
	enqueue(node2);	
	getMember(node2,state) = RUNNABLE;
	qHead=qHead->next;
	//printf("The thread specific data is(not set here) %d\n",((int*)(mythread_getspecific(testkey))));
	struct testD { int x; char b;} ts,ts2;
	ts.x=50; ts.b='l';
	mythread_setspecific(testkey,&ts);
	//int f=(*((struct testD *)(mythread_getspecific(testkey)))).x;
	//char z=((struct testD *)(mythread_getspecific(testkey)))->b;
	//printf("The values are int : %d char :%c\n",f,z);
	DNODE foundNode = search(2);
	qHead=qHead->prev;
	//printf("The thread specific data is %d\n",*((int*)(mythread_getspecific(testkey))));
	mythread_key_create(&testkey2,NULL);	
	ts2.x=12;
	ts2.b='m';
	mythread_setspecific(testkey2,(void*)(&ts2));
	//f=(*((struct testD *)(mythread_getspecific(testkey2)))).x;
	//z=((struct testD *)(mythread_getspecific(testkey2)))->b;
//	printf("The values are : for testkey2 int : %d char :%c\n",f,z);
	printf("The thread specific data is(IS set here) :testkey2: %u\n",((unsigned int*)(mythread_getspecific(testkey2))));
//	qHead=qHead->next;
//	printf("The thread specific data is(not set here) :testkey2: %d\n",((int*)(mythread_getspecific(testkey2))));
//	printf("id of foundNode : %d \n" , getMember(foundNode,threadId));
	printf("The thread specific data is %d\n",*((int*)(mythread_getspecific(testkey))));
	qHead=qHead->next;
	int f=(*((struct testD *)(mythread_getspecific(testkey)))).x;
	char z=((struct testD *)(mythread_getspecific(testkey)))->b;
	printf("The values are int : %d char :%c\n",f,z);
	mythread_key_delete(testkey);
	printf("The thread specific data is(not set here) :testkey2: %d\n",((int*)(mythread_getspecific(testkey))));
	qHead=qHead->next;
	printf("The thread specific data is(not set here):node 2 :testkey2: %d\n",((int*)(mythread_getspecific(testkey))));
	qHead=node1;
	f=(*((struct testD *)(mythread_getspecific(testkey2)))).x;
	z=((struct testD *)(mythread_getspecific(testkey2)))->b;
	printf("The values are : for testkey2 int : %d char :%c\n",f,z);
	printf("The sanity of key1 is %d\n",key_arr[0].sane);
	printf("The sanity of key2 is %d\n",key_arr[1].sane);
		
	mythread_key_delete(testkey2);	
	
	DNODE node3 = createNode();
	getMember(node3,threadId) = 3;
	getMember(node3,state) = RUNNABLE;
	enqueue(node3);
	qHead=node3;
	mythread_key_create(&testkey,NULL);
	*testdata=79
	mythread_setspecific(testkey,(void *)testdata);	
	printf("value for node 3 in key: %d is %d\n",testkey,*((int *)(mythread_getspecific(testkey))));	
*/

/*
	qHead = qHead->next;
	printQ();
	foundNode  = searchRunnable();
	printf(" id of runnable Node : %d \n" , getMember(foundNode,threadId));

	dequeue();
	printQ();

	dequeue();
	printQ();
	
*/
	
	int tid1,tid2,tid3;
	mythread_create(&tid1,NULL,sayHello,NULL);
	//mythread_join(tid1,NULL);
	mythread_create(&tid2,NULL,sayHello2,NULL);

	int tmp;
	//mythread_join(tid2,NULL);
	
//	mythread_create(&tid3,NULL,sayHello3,NULL);

//	mythread_join(tid3,NULL);
/*
//	mythread_create(&tid1,NULL,sayHello,NULL);
//	sleep(9);
	mythread_create(&tid1,NULL,sayHello6,NULL);
*/
	printOut("All threads atleast enqueued\n");
	mythread_exit(NULL);
//	waitpid(tid1,0,0);
//	sleep(1);	
}

