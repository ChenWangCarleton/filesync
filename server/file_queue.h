#ifndef __FILE_QUEUE_H
#define __FILE_QUEUE_H




struct QNode{
	char fn[50];
	int purpose;//0 for write, 1 for delete
    struct QNode *next;
};

typedef struct QNode * bag;


struct Queue{
	int capacity;
    struct QNode *front, *rear;
};
 
typedef struct Queue * bags;

bag newNode(char k[],int p);
bags createQueue();
int enQueue(bags q, char k[],int p);
bag deQueue(bags q);
int compareQ(bag a, bag b);
#endif
