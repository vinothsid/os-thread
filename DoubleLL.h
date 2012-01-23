#include<stdio.h>
#include<malloc.h>
#define print(string) write(1,string,sizeof(string))

struct dllNode {
	void *data;
	struct dllNode *next;
	struct dllNode *prev;
};

typedef struct dllNode * DNODE ;


DNODE getDllNode (void *data) ;
int dllInsert(DNODE prevNode , DNODE newNode);
int dllDelete(DNODE n,DNODE *head);
int insertAtEnd(DNODE head , DNODE newNode);

// data should be address of the variable containing the data . Value cannot be passed directly
DNODE find(DNODE head,int (*compare)(DNODE ,void *) , void *data); /*
	fp should return 1 on success and 0 on failure
	Example for compare :
		int compareThreadId(struct node *n , void *d) {
			int id = *(int *)d;
			if(getMember(n,threadId) == id)
				return 1;
			else
				return 0;

		}

			int (*fp)(struct node *,void *);
			fp = compareThreadId;

		fp(someNode,&someData);

*/
