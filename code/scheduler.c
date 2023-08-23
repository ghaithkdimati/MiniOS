#include "headers.h"

int PG_S_shmid;//shared memory id between process generator and scheduler
struct processData* PG_S_shmaddr;//address of PG_S_shmid
int prevID=-1;//used to check if memory changed
struct PCB runningProcess;

void childHandler()
{
    runningProcess.id=-1;
}


void HPF()
{
    struct PriorityQueue processesPQ; 
    struct PCB newProcess;
    int PID;
    runningProcess.id=-1;
    double waitingSum=0;
    double turnaroundSum=0;
    int sumProcesses=0;
    while(1){
        //scheduling incoming new processes



        //data for running process
        
        
        if((*PG_S_shmaddr).id!=prevID)
        {
            printf("arrived ID=%d, running time=%d\n",(*PG_S_shmaddr).id,(*PG_S_shmaddr).runningtime);
            prevID=(*PG_S_shmaddr).id;
            newProcess.priority=(*PG_S_shmaddr).priority;
            newProcess.remaingTime=(*PG_S_shmaddr).runningtime;
            newProcess.id=(*PG_S_shmaddr).id;
            newProcess.state='W';
            newProcess.arrivalTime=(*PG_S_shmaddr).arrivaltime;
            priorityEnqueue(&processesPQ,newProcess,newProcess.priority);

        }
        //printf("runningProcessID=%d\n",runningProcess.id);
        if(runningProcess.id==-1)
        {
            struct PCB* tempProcess=priorityDequeue(&processesPQ);
            if(tempProcess)
            {
                runningProcess=*tempProcess;
                char runTime=runningProcess.remaingTime;
                char* runtimeAddress=&runTime;
                int PID=fork();
                runningProcess.PID=PID;
                printf("PID=%d started\n",runningProcess.PID);
                printf("----------------PID=%d------------------\n",PID);
                printf("----------------PID=%d------------------\n",PID);
                if(PID==0)
                {
                    execv("./process",&runtimeAddress);
                    
                }
                // runningProcess.state='R';
                // runningProcess.waitingTime=getClk()-runningProcess.arrivalTime;
                // waitingSum+=runningProcess.waitingTime;
            }
        }
        // else
        // {
        //     // int status;
        //     // printf("childPID=%d\n",runningProcess.PID);
        //     // pid_t flagWait=waitpid(runningProcess.PID,&status,WNOHANG);
        //     // if(flagWait==0)
        //     //     printf("not finished\n");
        //     // if(flagWait!=0);
        //     // {
        //     //     printf("PID=%d finished\n",runningProcess.PID);
        //     //     runningProcess.id=-1;
        //     //     turnaroundSum+=(getClk()-runningProcess.arrivalTime);
        //     // }
        // }
    }
}

void SRTN()
{
struct PriorityQueue processes;
struct PCB newProcess;
int prevID=-1;
while(1)
{
if(*(PG_S_shmaddr).id!=prevID)
{
            printf("arrived ID=%d, running time=%d\n",(*PG_S_shmaddr).id,(*PG_S_shmaddr).runningtime);
            prevID=(*PG_S_shmaddr).id;
            newProcess.priority=(*PG_S_shmaddr).priority;
            newProcess.remaingTime=(*PG_S_shmaddr).runningtime;
            newProcess.id=(*PG_S_shmaddr).id;
            newProcess.state='W';
            newProcess.arrivalTime=(*PG_S_shmaddr).arrivaltime;
            priorityEnqueue(&processesPQ,newProcess,newProcess.priority);

}





}
}
int main(int argc, char * argv[])
{
    //initClk();
    signal(SIGCHLD,childHandler);
    printf("%c\n",*argv[0]);

    //------------------Creating a shm with Scheduler------------------//
    PG_S_shmid=shmget(SHKEY_PG_S,sizeof(struct processData),IPC_CREAT | 0644);
    if ((long)PG_S_shmid == -1)
    {
        perror("Error in creating shm between process generator and sched!\n");
        exit(-1);
    }
    PG_S_shmaddr = shmat(PG_S_shmid, (void *)0, 0);
    if ((long)PG_S_shmaddr == -1)
    {
        perror("Error in attaching the shm  between process generator and sched!\n");
        exit(-1);
    }

    if(*argv[0]=='1')
        HPF();
    
    //upon termination release the clock resources.


    //destroyClk(true);
}

