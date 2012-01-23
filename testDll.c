#include<stdio.h>
#include"DoubleLL.h"

#define getMember(node,member) ((struct thread *)((node)->data))->member

struct thread {
        int threadId;
};

int printDll(DNODE head ) {
	if ( head!= NULL) {
		DNODE p = head;
		do {
			printf("Thread id : %d \n " , getMember(p,threadId));
			p = p->next;
		} while(p!=head);
		return 0;
	} else {
		print("head is NULL\n");
		return -1;

	}
}

int compareThreadId( DNODE n , void *d) {
        int id = *(int *)d;
	printf("Comparing %d and %d\n",getMember(n,threadId),id);
        if(getMember(n,threadId) == id)
                return 1;
        else
                return 0;

}

int main () {
	struct thread t1;
	t1.threadId = 1;

	DNODE node1 = getDllNode(&t1);

	DNODE head = node1;
	//printDll(head);
//	dllDelete(&head);

	struct thread t2;
	t2.threadId = 2;
	DNODE node2 = getDllNode(&t2);

	insertAtEnd(head,node2);
	printDll(head);

	printf(" head of prev : %d \n", getMember(head->prev,threadId));

	struct thread t3;
	t3.threadId = 3;
	DNODE node3 = getDllNode(&t3);
	insertAtEnd(head,node3);
	printDll(head);

	struct thread t4;
	t4.threadId = 4;
	DNODE node4 = getDllNode(&t4);
	dllInsert(node1,node4);
	printDll(head);


/*
	dllDelete(node2,&head);
	dllDelete(node1,&head);
	dllDelete(node3,&head);
	dllDelete(node4,&head);
*/

//	printf("After delete : \n");
//	printDll(head);	
	printf("Searching for tid \n");
	int t = 2;
	void *y = &t ;
	DNODE tmp = find(head,compareThreadId,&t);
	printDll(tmp);









}
