#include "mythread.h"

int main() {
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

}
