#include "PriorityQueue.h"






struct floatNode { 
    float val;
    struct floatNode* next;
};


struct floatQueue
{
    struct floatNode* Head;
    struct floatNode* Tail;
    int count;
};

void floatQueueInit(struct floatQueue* q)
{
    q->count=0;
    q->Head=NULL;
    q->Tail=NULL;
}



void floatEnqueue(struct floatQueue* q,float data)
{
    struct floatNode* n=(struct floatNode*)malloc(sizeof(struct floatNode));
    n->val=data;
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

float* floatDequeue(struct floatQueue* q)
{
    if(q->count==0)
        return NULL;
    
    float* p;
    p=(float*)malloc(sizeof(float));
    *p=q->Head->val;
    struct floatNode* tmp=q->Head;

    q->count--;
    q->Head=q->Head->next;
    free(tmp);
    return p;
}


void floatPrint(struct floatQueue* q)
{
    struct floatNode* n=q->Head;
    while(n)
    {
        printf("val= %f\n",n->val);
        n=n->next;
    }
}