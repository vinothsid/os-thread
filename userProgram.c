//#include "mythread.h"

void* sayHello() {
	print("I say hello\n");
}

void* sayHello2() {
	print("I say hello2\n");
}

void* sayHello3() {
	print("I say hello3\n");
}

int main() {
	/*
	DNODE node1 = createNode();
	getMember(node1,threadId) = 1;
	qHead = node1;

	DNODE node2 = createNode();
	getMember(node2,threadId) = 2;
	enqueue(node2);	
		
	printQ();

	DNODE foundNode = search(2);
	printf("id of foundNode : %d \n" , getMember(foundNode,threadId));

	DNODE node3 = createNode();
	getMember(node3,threadId) = 3;
	getMember(node3,state) = RUNNABLE;
	enqueue(node3);

	foundNode  = searchRunnable();
	printf("id of runnable Node : %d \n" , getMember(foundNode,threadId));

	dequeue();
	printQ();

	dequeue();
	printQ();
	*/
	int tid1,tid2,tid3;
	mythread_create(&tid1,NULL,sayHello,NULL);
	waitpid(tid1,0,0);
	
}
