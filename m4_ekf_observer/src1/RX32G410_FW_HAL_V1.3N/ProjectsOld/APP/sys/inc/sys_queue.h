#ifndef SYS_QUEUE_H_
#define SYS_QUEUE_H_
#include <stdbool.h>

#define MAXQSIZE 200
#define OK 1
#define ERROR 0
#define OVERFLOW -1

typedef char  Item;

typedef struct {
	Item *base;
	Item front;
	Item rear;
}QueueDef;

/*initialize the queue*/
void InitQueue(QueueDef *q);

/*return the length of the queue*/
unsigned int QueueLength(QueueDef q);

/*Destroy the queue*/
void DestroyQueue(QueueDef *q);

/*determine if the queue is empty*/
bool IsEmpty(QueueDef q);

/*determine if the queue is full*/
bool IsFull(QueueDef q);

/*return the head elem of the queue*/
Item Top(QueueDef q);

/*return the back elem of the queue*/
Item Back(QueueDef q);

/*enqueue, insert the rear*/
bool EnQueue(QueueDef *q, Item e);

/*dequeue, pop the front*/
bool DeQueue(QueueDef *q,Item* buff);

/*print the queue*/
void PrintQueue(QueueDef q);

char	UartPopQueue(char*	buff);
#endif



