#include "headers.h"



int main()
{
    struct PriorityQueue q;
    struct PCB p;
    p.executionTime=1;
    p.id=1;
    p.priority=3;
    p.remaingTime=3;
    p.state='R';
    p.waitingTime=3;

    priorityEnqueue(&q,p,p.priority);
    p.id=2;
    p.priority=1;
    priorityEnqueue(&q,p,p.priority);
    p.id=3;
    p.priority=2;
    priorityEnqueue(&q,p,p.priority);
    p.id=4;
    p.priority=4;
    priorityEnqueue(&q,p,p.priority);
    p.id=5;  
    p.priority=3;
    priorityEnqueue(&q,p,p.priority);
    
    priorityPrint(&q);
    printf("------------------------------------\n");
    struct PCB* p1;
    p1=priorityDequeue(&q);
    printf("id=%d\n",p1->id);

    printf("------------------------------------\n");

    p1=priorityDequeue(&q);
    printf("id=%d\n",p1->id);

    printf("------------------------------------\n");p1=priorityDequeue(&q);
    printf("id=%d\n",p1->id);

    printf("------------------------------------\n");

    
    priorityPrint(&q);   

}