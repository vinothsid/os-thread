/*
 * mythread.h -- interface of user threads library
 */

#ifndef MYTHREAD_H
#define MYTHREAD_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <sys/syscall.h>
#include <sys/types.h>
#include "futex.h"
#include "DoubleLL.h"

#define RUNNABLE 1
#define DEAD 0
#define WAIT 2
#define CREATED 3

#define STACK_SIZE 64*1024
#define getMember(node,member) ((struct thread *)((node)->data))->member

/* Keys for thread-specific data */
typedef unsigned int mythread_key_t;
typedef unsigned int mythread_attr_t;
typedef unsigned int mythread_t;
typedef unsigned int mythread_attr_t;

DNODE qHead, idleNode;
struct futex queueLock; //to ensure thread queue is not modified by two thread simultaneously;
int INIT_ONCE;
struct futex schedulerLock;
/* add your code here */

struct thread {
	int state; //CREATED,RUNNABLE, WAIT, DEAD;;
	int threadId;
	struct futex selfLock;
	char* stackPtr; //needed to free stack later;
	DNODE joinQ; //queue of threads waiting on this thread;
	int exitStatus;
};

struct task {
	void* (*func)();//user func to be executed;
	void *arg;    //arguments for user func;
	DNODE qPos;//position in the thread queue; pointer to self;
};
/*
*call to get process id or thread id
*/

static pid_t gettid(void);

/*
*thread library initialization
*initializes queueLock,scheduler thread and idle thread
*/
int initThread();
/*
*this function is used as a wrapper to call the user function
*/
int mythread_wrapper(void *);

/*
*scheduler thread: schedules next thread to run. It schedules an idle thread when 
*there is no ready process
*/
int scheduler();

/*
*idle thread: scheduled by scheduler when there is no active thread
*/
int idleFunc();

/*
* Enqueues thread in the run Q
*/
int enqueue(DNODE);

/*
* Creates and initialises the node to be joined in run Q
* initializes the struct thread associated with each thread started
* by the mythread library
* sets state as CREATED in the thread struct
*/
DNODE createNode();

/*
* Deques the node in the head of run Q
*/
int dequeue();

/*
* Search for the threadId in the run Q and return the node
*/
DNODE search(int threadId);

/*Compare helper function for search()
*
*/
int compare(DNODE,void *);

/*
* Search for first Runnable thread in the run Q 
*/
DNODE searchRunnable();

/* Helper function for searchRunnable
*
*/
int compareState(DNODE n,void *d);

/*
* Sets the state of all threads in Join Q RUNNABLE 
*/
int wake();

/*
* Join the calling thread Node(pointed by head) to join Q of target thread 
*/
int joinQ(mythread_t );

/*
 * mythread_self - thread id of running thread
 */
mythread_t mythread_self(void);

/*
*
*/

/*
 * mythread_create - prepares context of new_thread_ID as start_func(arg),
 * attr is ignored right now.
 * Threads are activated (run) according to the number of available LWPs
 * or are marked as ready.
 */
int mythread_create(mythread_t *new_thread_ID, mythread_attr_t *attr, void * (*start_func)(void *), void *arg);

/*
 * mythread_yield - switch from running thread to the next ready one
 */
int mythread_yield(void);

/*
 * mythread_join - suspend calling thread if target_thread has not finished,
 * enqueue on the join Q of the target thread, then dispatch ready thread;
 * once target_thread finishes, it activates the calling thread / marks it
 * as ready.
 */
int mythread_join(mythread_t target_thread, void **status);

/*
 * mythread_exit - exit thread, awakes joiners on return
 * from thread_func and dequeue itself from run Q before dispatching run->next
 */
void mythread_exit(void *retval);

/*
 * mythread_key_create - thread-specific data key creation
 * The  mythread_key_create()  function shall create a thread-specific data
 * key visible to all threads in  the  process.  Key  values  provided  by
 * mythread_key_create()  are opaque objects used to locate thread-specific
 * data. Although the same key value may be used by different threads, the
 * values  bound  to  the key by mythread_setspecific() are maintained on a
 *per-thread basis and persist for the life of the calling thread.
 */
int mythread_key_create(mythread_key_t *key, void (*destructor)(void*));

/*
 * mythread_key_delete - thread-specific data key deletion
 * The  mythread_key_delete()  function shall delete a thread-specific data
 * key previously returned by  mythread_key_create().  The  thread-specific
 * data  values  associated  with  key  need  not  be  NULL  at  the  time
 * mythread_key_delete() is called.  It is the responsibility of the appli‐
 * cation  to  free any application storage or perform any cleanup actions
 * for data structures related to the deleted key  or  associated  thread-
 * specific data in any threads; this cleanup can be done either before or
 * after mythread_key_delete() is called. Any attempt to use key  following
 * the call to mythread_key_delete() results in undefined behavior.
 */
int mythread_key_delete(mythread_key_t key);

/*
 * mythread_getspecific - thread-specific data management
 * The mythread_getspecific() function shall  return  the  value  currently
 * bound to the specified key on behalf of the calling thread.
 */
void *mythread_getspecific(mythread_key_t key);

/*
 * mythread_setspecific - thread-specific data management
 * The  mythread_setspecific()  function  shall associate a thread-specific
 * value with a key obtained via a previous call to  mythread_key_create().
 * Different threads may bind different values to the same key. These val‐
 * ues are typically pointers to blocks of  dynamically  allocated  memory
 *that have been reserved for use by the calling thread.
 */
int mythread_setspecific(mythread_key_t key, const void *value);


#endif /* MYTHREAD_H */
