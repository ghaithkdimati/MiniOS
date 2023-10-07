#include "IntQueue.h"

struct PFloatNode { 
    float data;
    struct PFloatNode* next;
    int priority;
};


struct FloatPriorityQueue
{
    struct PFloatNode* Head;
    struct PFloatNode* Tail;
    int count;
};



void InitFloatPriorityQueue(struct FloatPriorityQueue * q)
{
    q->count=0;
    q->Head=NULL;
    q->Tail=NULL;
}



void floatPriorityEnqueue(struct FloatPriorityQueue * q,float data,int p)
{
    struct PFloatNode* n=(struct PFloatNode*)malloc(sizeof(struct PFloatNode));
    n->data=data;
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
    struct PFloatNode* tmp=q->Head;
    while (tmp->next && tmp->next->priority<=n->priority)
        tmp=tmp->next;
    
    n->next=tmp->next;
    tmp->next=n;
    q->count++;
}


float*  floatPriorityDequeue(struct FloatPriorityQueue * q)
{
    if(q->count==0)
        return NULL;
    
    float* p;
    p=(float*)malloc(sizeof(float));
    *p=q->Head->data;
    struct PFloatNode* tmp=q->Head;

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



void floatPriorityPrint(struct FloatPriorityQueue * q)
{
    struct PFloatNode* n=q->Head;
    while(n)
    {
        printf("id= %d, priority=%d\n",(int)n->data,(int)n->priority);
        n=n->next;
    }
}

