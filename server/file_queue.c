#include "file_queue.h"
#include <stdlib.h>
#include<string.h>


bag newNode(char k[],int p){
    bag temp = (bag)malloc(sizeof(struct QNode));
	strcpy(temp->fn,k);   
	temp->purpose=p;
    temp->next = NULL;
    return temp; 
}
 
bags createQueue(){
    bags q = (bags)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
	q->capacity=0;
    return q;
}
 
int enQueue(bags q, char k[],int p){
	if(q->capacity>1){
		if(strcmp(q->rear->fn,k)==0&&q->rear->purpose==p){
			return -1;
		}
	}
    bag temp = newNode(k,p);
 	q->capacity++;
    if (q->rear == NULL){
       q->front = q->rear = temp;
       return 0;
    }

    q->rear->next = temp;
    q->rear = temp;
	return 0;
}
 
bag deQueue(bags q){
	q->capacity--;

    if (q->front == NULL)return NULL;
 
    bag temp = q->front;
    q->front = q->front->next;
	
    if (q->front == NULL)q->rear = NULL;
    return temp;
}
int compareQ(bag a, bag b){
	if(strcmp(a->fn,b->fn)==0&&a->purpose==b->purpose)
		return 0;
	return -1;
}