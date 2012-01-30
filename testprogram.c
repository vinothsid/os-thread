#include<stdio.h>
#include"mythread.h"

mythread_key_t key1;
mythread_key_t key2;
void *func(void *name) {

	printOut((char*)name);
}

void *thread1() {

	printOut(itoa(mythread_self(),10));
	printOut(" Thread1 running\n");
}

void *thread2() {
	printOut(itoa(mythread_self(),10));
	printOut(" Thread2 running\n");

}

void *thread3() {
	printOut(itoa(mythread_self(),10));
	printOut(" Thread3 running\n");

}

void getKeyValue() {
	char *data = (char *)mythread_getspecific(key1);
	printOut("Key Value retrieved in thread :");
	printOut(itoa(mythread_self(),10));
	printOut("Value : " );
	if(data!=NULL)
		printOut(data);
	else
		printOut("Key value not found\n");
	printOut("\n");

}

void deleteKey() {
	mythread_key_delete(key1);

	printOut("After key delete\n");
	char *data = (char *)mythread_getspecific(key1);
	if(data==NULL)
		printOut("Data is NULL\n");
	else
		printOut("This should not be printed\n");

}
void *yieldThread1() {
	static int i = 1;

	sleep(1); //Sleeping for 1 second so that the next thread joins the queue

	printOut("Creating key1 in thread : ");
	printOut(itoa(mythread_self(),10));
	printOut("\n");
	mythread_key_create(&key1,NULL);

	char *d = malloc(100);
	strcpy(d,"yieldThread1 value");
	mythread_setspecific(key1,(void *)d);
	while(i<=3) {
		printOut(itoa(mythread_self(),10));
		printOut(" yieldThread1 yielding #");
		printOut(itoa(i,10));
		printOut("\n");
		mythread_yield();
		i++;
	}
	

	
	getKeyValue();
		
}

void *yieldThread2() {
	static int i = 1;
	printOut("Creating key1 in thread : ");
	printOut(itoa(mythread_self(),10));
	printOut("\n");
	//mythread_key_create(&key1,NULL);
	char *d = malloc(100);
	strcpy(d,"yieldThread2 value");
	mythread_setspecific(key1,(void *)d);
	while(i <= 3) {
		printOut(itoa(mythread_self(),10));
		printOut(" yieldThread2 yielding #");
		printOut(itoa(i,10));
		printOut("\n");
		mythread_yield();
		i++;
	}
		
	getKeyValue();
	
	deleteKey();
}

void *exitThread() {
	
	printOut("calling mythread_exit\n");
	mythread_exit(NULL);
	printOut("This should not be printed\n");
}

int main() {

	mythread_t tid1,tid2,tid3,tid4;

	mythread_create(&tid1,NULL,thread1,NULL);
	mythread_create(&tid2,NULL,thread2,NULL);
	mythread_create(&tid3,NULL,thread3,NULL);


	mythread_join(tid1,NULL);
	mythread_join(tid2,NULL);
	mythread_join(tid3,NULL);

	mythread_create(&tid4,NULL,func,(void *)"first 3 threads joined. Argument passed successfully\n");
	mythread_join(tid4,NULL);

	mythread_create(&tid1,NULL,yieldThread1,NULL);
	mythread_create(&tid2,NULL,yieldThread2,NULL);
	mythread_join(tid1,NULL);
	mythread_join(tid2,NULL);

	mythread_create(&tid3,NULL,exitThread,NULL);
	mythread_exit(NULL);
}
