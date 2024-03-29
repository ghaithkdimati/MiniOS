#include <stdlib.h>
#include <stdio.h>

struct PCB
{
    int priority;
    int id;
    int PID;
    int remaingTime;
    int executionTime;
    char state;
    int arrivalTime;
    int waitingTime;
    int lastStopped;
    int memSize;
    int memStart;
};


struct Node { 
    struct PCB process;
    struct Node* next;
};


struct Queue
{
    struct Node* Head;
    struct Node* Tail;
    int count;
};

void ProcessQueueInit(struct Queue* q)
{
    q->count=0;
    q->Head=NULL;
    q->Tail=NULL;
}

void enqueue(struct Queue* q,struct PCB data)
{
    struct Node* n=(struct Node*)malloc(sizeof(struct Node));
    n->process=data;
    if(q->Head==NULL)
    {
        q->Head=n;
        q->Tail=n;
        q->count++;
        return;
    }
    q->Tail->next=n;
    q->Tail=n;
    q->count++;
}


struct PCB* dequeue(struct Queue* q)
{
    if(q->count==0)
        return NULL;
    
    struct PCB* p;
    p=(struct PCB*)malloc(sizeof(struct PCB));
    *p=q->Head->process;
    struct Node* tmp=q->Head;

    q->count--;
    q->Head=q->Head->next;
    if(q->count==0)
    {
        q->Head=NULL;
        q->Tail=NULL;
    }
    free(tmp);
    return p;
}



void print(struct Queue* q)
{
    struct Node* n=q->Head;
    while(n)
    {
        printf("id= %d\n",n->process.id);
        n=n->next;
    }
}