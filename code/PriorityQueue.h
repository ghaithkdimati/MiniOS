#include "ProcessQueue.h"

struct Pnode { 
    struct PCB process;
    struct Pnode* next;
    int priority;
};


struct PriorityQueue
{
    struct Pnode* Head;
    struct Pnode* Tail;
    int count;
};



void InitPriorityQueue(struct PriorityQueue* q)
{
    q->count=0;
    q->Head=NULL;
    q->Tail=NULL;
}



void priorityEnqueue(struct PriorityQueue* q,struct PCB data,int p)
{
    struct Pnode* n=(struct Pnode*)malloc(sizeof(struct Pnode));
    n->process=data;
    n->priority=p;
    if(q->Head==NULL)
    {
        q->Head=n;
        q->count++;
        return;
    }
    if(p<q->Head->priority)
    {
        n->next=q->Head;
        q->Head=n;
        q->count++;
        return;
    }
    struct Pnode* tmp=q->Head;
    while (tmp->next && tmp->next->priority<=n->priority)
        tmp=tmp->next;
    
    n->next=tmp->next;
    tmp->next=n;
    q->count++;
}


struct PCB*  priorityDequeue(struct PriorityQueue* q)
{
    if(q->count==0)
        return NULL;
    
    struct PCB* p;
    p=(struct PCB*)malloc(sizeof(struct PCB));
    *p=q->Head->process;
    struct Pnode* tmp=q->Head;

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



void priorityPrint(struct PriorityQueue* q)
{
    struct Pnode* n=q->Head;
    while(n)
    {
        printf("id= %d, priority=%d\n",n->process.id,n->process.priority);
        n=n->next;
    }
}

