#include "DoubleLL.h"

DNODE getDllNode (void *data) {
	DNODE n = (DNODE)malloc(sizeof(struct dllNode));
	n->data = data;
	n->next = n;
	n->prev = n;
	return n;
}

int dllInsert(DNODE prevNode , DNODE newNode) {
	if ( prevNode!=NULL ) {
		newNode->next = prevNode->next;
		prevNode->next = newNode;
		newNode->prev = prevNode;
		newNode->next->prev = newNode;
		return 0; 
	} else {
		print("prevNode is NULL\n");
		return -1;
	}
}

// The n->data has to be deleted before calling dllDelete
int dllDelete(DNODE n , DNODE *head ) {
//	DNODE n = *node;
	if (n==NULL) {
		print("Node already NULL\n");
		return -1;
	}
	if (n->next == n ) {
                
                *head = NULL ; // n->next and n are same only when there is ony one element and head is pointing to it;

		//this is the only element
		print("This is the last element\n");
		free(n);
//		*node = NULL;
		return 0;
	} else {

		if(n == *head)
			*head = n->next;

		n->prev->next = n->next;
		n->next->prev = n->prev;
		free(n);
//		*node = NULL;
		return 1;
	}
}

int insertAtEnd(DNODE head , DNODE newNode) {
//	if(head!=NULL) {
//		DNODE tail = head->prev;
//		head->prev = newNode;
		return dllInsert(head->prev,newNode);	
//	} else {
//		print("head is NULL\n");
//	}

	
}

DNODE find(DNODE head,int (*compare)(DNODE ,void *) ,void *data  ) {
	if ( head !=NULL ) {
		DNODE p;
		p=head;
		do {
			if( compare(p,data) )
				return p;
			p = p->next;
		} while(p!=head);

		return NULL; // if function reaches here data is not found

	} else {
		print("find:: Head is NULL\n");
		return NULL;
	}
}

