#include "headers.h"

int PG_S_shmid;                   // shared memory id between process generator and scheduler
int S_P_shmid;
struct processData *PG_S_shmaddr; // address of PG_S_shmid
int prevID = -1;                  // used to check if memory changed
struct PCB runningProcess;
int PID;
int PG_S_msgqid;
int* S_P_shmaddr;
FILE *outputFile;
FILE *outputFileMem;
struct floatQueue WTAqueue;
float WTASum=0;
float waitingSum = 0;
int memManagementAlgo;
bool SIGUSR1INT;

struct FloatPriorityQueue memQueueArr[9]; //Datastructre used for buddy algorith
bool mem[1024];                   //Free bit map for first Fit algorith
struct Queue waitingList;
int  totalAllocetedMemory;
int allocatedProcessID;
int totalProcessesRuntime=0;
int totalProcesses;
int totalFinishedProcesses;

//------------------Memory management methods------------------//
int firstFitAllocate(int size)
{
    int c=0;
    //------------------looping on the free bit map to check whether there is enough memory------------------//
    for(int i=0;i<1024;i++)
    {
        c=0;

        while(mem[i]==0 && i<1024 && c!=size)
        {
            c++;
            i++;
        }
        if(c==size)
        {
            for(int j=i-size;j<i;j++)
            {
                mem[j]=1;
            }
            printf("allocated memory from %d to %d\n",i-size,i);
            fprintf(outputFileMem,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),size,allocatedProcessID,i-size,i);
            totalAllocetedMemory+=size;
            return (i-size);
        }
    }
    return -1;
}
bool firstFitCanAllocate(int size)
{
    int c=0;
    for(int i=0;i<1024;i++)
    {
        c=0;

        while(mem[i]==0 && i<1024 && c!=size)
        {
            c++;
            i++;
        }
        if(c==size)
            return true;
    }
    return false;
}
void firstFitDeallocate(int start,int size)
{
    for(int i=start;i<start+size;i++)
        mem[i]=0;
    printf("deallocated memory from %d to %d\n",start,start+size);
    fprintf(outputFileMem,"At time %d freed %d bytes from process %d from %d to %d\n",getClk(),size,runningProcess.id,start,start+size);
    totalAllocetedMemory-=size;
}
int buddyAllocate(int size)
{
    int pos=ceil(log2(size));
    int i=pos;
    while(memQueueArr[i].count==0 && i<9)
    {
        i++;
    }
    if(i>=9)
        return -1;
    //------------------partitioning until the required memory size is found ------------------//

    while(i>pos)
    {
        
        int mem1=(int)*floatPriorityDequeue(&memQueueArr[i]);
        printf("dequeued from Queue%d start %d\n",i,mem1);
        int mem2=mem1+pow(2,i-1);
        i--;
        printf("enqueued to Queue%d start %d\n",i,mem1);  
        floatPriorityEnqueue(&memQueueArr[i],mem1,mem1);
        printf("enqueued to Queue%d start %d\n",i,mem2);  
        floatPriorityEnqueue(&memQueueArr[i],mem2,mem2);
    }
    int start=(int)*(floatPriorityDequeue(&memQueueArr[pos]));
    printf("finished buddy Allocationxxxxxxxxx\n");
    fprintf(outputFileMem,"At time %d allocated %d bytes for process %d from %d to %d\n",getClk(),size,allocatedProcessID,start,start+(int)pow(2,pos));
    totalAllocetedMemory+=pow(2,pos);
    printf("finished buddy Allocationxxxxxxxxx\n");
    return start;
}
void buddyDeallocate(int start, int size)
{
    int pos=ceil(log2(size));
    int size1=size;
    size=pow(2,pos);
    totalAllocetedMemory-=size;
    fprintf(outputFileMem,"At time %d freed %d bytes from process %d from %d to %d\n",getClk(),size1,runningProcess.id,start,start+size);
    int dir=((start/size)%2==0)?1:-1;
    int target=start+dir*size;
    int temp;
    bool flag=true;
    if(pos==8)
    {
        floatPriorityEnqueue(&memQueueArr[pos],start,start);
        return;
    }
    //------------------the larger loop traverses the memQueueArr which contains all the memory segmants queues------------------//
    while(flag && pos<8)
    {
        struct FloatPriorityQueue tempQueueMem;
        InitFloatPriorityQueue(&tempQueueMem);
        dir=((start/size)%2==0)?1:-1;
        target=start+dir*size;
        flag=false;
        int memQueueArrPosCount=memQueueArr[pos].count;
        //------------------the smaller loop traveses the available segmants in the current memQueue and looks for the segmant that can be------------------//
        //------------------merged with the recently deallocated or last merged segmant ------------------//

        for(int i=0;i<memQueueArrPosCount;i++)
        {

            temp=*floatPriorityDequeue(&memQueueArr[pos]);
            if(temp==target)
            {
                printf("Merged size:%d start:%d target:%d\n",size,start,target);
                flag=true;
                pos++;
                if(dir==-1)
                    start=target;
                size*=2;
                break;
            }
            else
                floatPriorityEnqueue(&tempQueueMem,temp,temp);
        }
        //------------------returning the unmerged segments to their original memQueue------------------//
        while(tempQueueMem.count>0)
        {
            temp=*floatPriorityDequeue(&tempQueueMem);
            if(flag)
                floatPriorityEnqueue(&memQueueArr[pos-1],temp,temp);
            else
                floatPriorityEnqueue(&memQueueArr[pos],temp,temp);            
        }


        //------------------checking whether further merging can occur------------------//
        if(!flag || size==256)
        {
            printf("cant merge enqueued to Queue%d start %d\n",pos,start);  
            floatPriorityEnqueue(&memQueueArr[pos],start,start);
        }
    }


}
bool buddyCanAllocate(int size)
{
    int pos=ceil(log2(size));
    while(pos<=8)
    {
        if(memQueueArr[pos].count!=0)
            return true;
        pos++;
    }
    return false;
}
int allocate(int size)
{
    if(memManagementAlgo==1)
        return firstFitAllocate(size);
    else
        return buddyAllocate(size);
}
void deallocate(int start,int size)
{
    if(memManagementAlgo==1)
        firstFitDeallocate(start,size);
    else
        buddyDeallocate(start,size);
}
bool canAllocate(int size)
{
    if(memManagementAlgo==1)
        return firstFitCanAllocate(size);
    else
        return buddyCanAllocate(size);
}



//------------------Signal Handlers------------------//
void childHandler()
{
    totalProcessesRuntime+=runningProcess.executionTime;
    totalFinishedProcesses++;
    SIGUSR1INT=true;
    printf("PID of child:%d\n", runningProcess.PID);
    int status;
    int clk=getClk();
    deallocate(runningProcess.memStart,runningProcess.memSize);
    int TA=clk-runningProcess.arrivalTime;
    float value = ((float)(TA))/runningProcess.executionTime;
    float WTA = ((int)(value * 100 + .5) / 100.0);
    WTASum+=WTA;
    waitingSum+=runningProcess.waitingTime;
    floatEnqueue(&WTAqueue,WTA);
    fprintf(outputFile,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",clk,runningProcess.id,runningProcess.arrivalTime,runningProcess.executionTime,0,runningProcess.waitingTime,TA,WTA);
    runningProcess.id = -1;
    wait(&status);
}
void exitHandler()
{

    int count=WTAqueue.count;
    floatPrint(&WTAqueue);
    float avgWTA=WTASum/count;
    float avgWaiting=waitingSum/count;
    float stdWTA=0;
    for(int i=0;i<count;i++)
    {
        float x=*floatDequeue(&WTAqueue);
        stdWTA+=((x-avgWTA)*(x-avgWTA));
    }
    stdWTA/=(count-1);
    stdWTA=sqrt((double)stdWTA);
    float util=(100*totalProcessesRuntime)/(float)(getClk());
    FILE* averageFile=fopen("scheduler.perf","w");
    util=((int)(util * 100 + .5) / 100.0);
    fprintf(averageFile,"CPU utilization = %.2f%% \n",util);
    avgWTA=((int)(avgWTA * 100 + .5) / 100.0);
    fprintf(averageFile,"Avg WTA = %.2f\n",avgWTA);
    avgWaiting=((int)(avgWaiting * 100 + .5) / 100.0);
    fprintf(averageFile,"Avg Waiting = %.2f\n",avgWaiting);
    stdWTA=((int)(stdWTA * 100 + .5) / 100.0);
    fprintf(averageFile,"Std WTA == %.2f\n",stdWTA);
    
    

    fclose(averageFile);
    fclose(outputFile);
    fclose(outputFileMem);
    exit(0);
}


struct msgBuffDummy
{
    char x[1];
    int mtype;
};


//------------------Scheduling Algorithms------------------//
void HPF()
{
    struct PriorityQueue processesPQ;
    InitPriorityQueue(&processesPQ);
    struct PCB newProcess;
    runningProcess.id = -1;
    double turnaroundSum = 0;
    int sumProcesses = 0;
    //(*PG_S_shmaddr).id = -1;
    // prevID=-1;
    struct msgBuffDummy dummy;
    dummy.mtype=0;
    dummy.x[1]='c';

    struct processData tempPD; 
    while (1)
    {
        
        //------------------Recieving arriving processes from process generator------------------//
        while ((*PG_S_shmaddr).id != prevID)
        {
            tempPD = *PG_S_shmaddr;
            if (-1 == msgsnd(PG_S_msgqid, &dummy, sizeof(dummy), IPC_NOWAIT))
                printf("error happened in recv\n");
            printf("arrived ID=%d, running time=%d\n", tempPD.id, tempPD.runningtime);
            prevID = tempPD.id;
            newProcess.priority = tempPD.priority;
            newProcess.remaingTime = tempPD.runningtime;
            newProcess.id = tempPD.id;
            newProcess.state = 'W';
            newProcess.arrivalTime = tempPD.arrivaltime;
            newProcess.arrivalTime;
            newProcess.executionTime=tempPD.runningtime;
            newProcess.memSize=tempPD.memSize;
            priorityEnqueue(&processesPQ, newProcess, newProcess.priority);
            usleep(1);
        }
        //------------------Starting the scheduled process------------------//
        if (runningProcess.id == -1)
        {
            struct PCB *tempProcess = priorityDequeue(&processesPQ);
            
            
            if (tempProcess)
            {
                printf("priority of running process: %d\n", tempProcess->priority);
                runningProcess = *tempProcess;
                char runTime = runningProcess.remaingTime;
                //------------------allocating memory before it starts------------------//
                runningProcess.memStart=allocate(runningProcess.memSize);
                char *runtimeAddress = &runTime;
                int PID = fork();
                if (PID == 0)
                {
                    //printf("lmaoooooooooo\n");
                    int check1 = execl("./process", runtimeAddress, NULL);
                    if (check1 == -1)
                        printf("unsuccessful execv with error%d\n", errno);
                    printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
                }
                runningProcess.PID = PID;
                runningProcess.state = 'R';
                runningProcess.arrivalTime;
                runningProcess.waitingTime = getClk() - runningProcess.arrivalTime;
                fprintf(outputFile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),runningProcess.id,runningProcess.arrivalTime,runningProcess.executionTime,runningProcess.remaingTime,runningProcess.waitingTime);
            }
        }
    }
}
void SRTN()
{
    struct PriorityQueue Processes;
    InitPriorityQueue(&Processes);
    int prevClk = getClk();
    runningProcess.id = -1;
    prevID = -1;
    struct PCB newProcess;
    struct msgBuffDummy dummy;
    dummy.mtype = 0;
    dummy.x[1] = 'c';
    struct processData tempPD;
    struct PCB *tempPCBMem;
    SIGUSR1INT=false;
    while (1)
    {
        int waitingListCount=waitingList.count;
        if(SIGUSR1INT)
        {
            for(int i=0;i<waitingListCount;i++)
            {
                tempPCBMem=dequeue(&waitingList);
                if(tempPCBMem && canAllocate(tempPCBMem->memSize))
                {
                    printf("tempPCBMemID=%d ,memSize=%d, i=%d\n",tempPCBMem->id,tempPCBMem->memSize,i);
                    printf("allocated********************\n");
                    allocatedProcessID=tempPCBMem->id;
                    tempPCBMem->memStart=allocate(tempPCBMem->memSize);
                    priorityEnqueue(&Processes,*tempPCBMem,tempPCBMem->remaingTime);
                }
                else
                    enqueue(&waitingList,*tempPCBMem);
                printf("freed tempPCBMemID=%d\n",tempPCBMem->id);
                free(tempPCBMem);
            }
            printf("waitingListCount=%d\n",waitingList.count);
            SIGUSR1INT=false;
        }


        //------------------Recieving arriving processes from process generator------------------//
        while ((*PG_S_shmaddr).id != prevID)
        { 
            tempPD = *PG_S_shmaddr;
            if (-1 == msgsnd(PG_S_msgqid, &dummy, sizeof(dummy), !IPC_NOWAIT))
                printf("error happened in recv\n");
            printf("arrived ID=%d, running time=%d\n", (*PG_S_shmaddr).id, (*PG_S_shmaddr).runningtime);
            struct PCB newProcess;
            newProcess.arrivalTime = tempPD.arrivaltime;
            newProcess.id = tempPD.id;
            newProcess.priority = tempPD.priority;
            newProcess.remaingTime = tempPD.runningtime;
            newProcess.executionTime=tempPD.runningtime;
            newProcess.memSize=tempPD.memSize;
            newProcess.state='W';
            newProcess.memStart=-1;
            if(waitingList.count>0 && !canAllocate(newProcess.memSize))
                enqueue(&waitingList,newProcess);
            else
                priorityEnqueue(&Processes,newProcess,newProcess.remaingTime);
            prevID = newProcess.id;
            
            usleep(1);
        }
        //------------------Starting the scheduled process------------------//
        if (runningProcess.id == -1 && !SIGUSR1INT)
        {
            struct PCB *processPtr;
            
            processPtr = priorityDequeue(&Processes);
            if (processPtr )
            {

                runningProcess = *processPtr;
                free(processPtr);
                //------------------Checking whether the process has been forked------------------//
                if (runningProcess.state == 'W')
                {
                  //------------------Checking whether the process has already been allocated and whether it can be allocated------------------//
                    if(runningProcess.memStart==-1)
                    {
                        allocatedProcessID=runningProcess.id;
                        runningProcess.memStart=allocate(runningProcess.memSize);
                    }
                    //------------------adding the processes that can't be allocated to the waiting list------------------//
                    if(runningProcess.memStart==-1)
                    {
                        enqueue(&waitingList,runningProcess);
                        runningProcess.id=-1;
                    }
                    else{
                        runningProcess.waitingTime = getClk() - runningProcess.arrivalTime;
                        char runTime = runningProcess.remaingTime;
                        char *runtimeAddress = &runTime;
                        int PID = fork();
                        if (PID == 0)
                        {
                            int check1 = execl("./process", runtimeAddress, NULL);
                            if (check1 == -1)
                                printf("unsuccessful execv with error%d\n", errno);
                        }
                        prevClk=getClk();
                        runningProcess.PID = PID;
                        runningProcess.state = 'R';
                        printf(" a process started ID= %d , PID = %d , \n", runningProcess.id, runningProcess.PID);
                        fprintf(outputFile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),runningProcess.id,runningProcess.arrivalTime,runningProcess.executionTime,runningProcess.remaingTime,runningProcess.waitingTime);
                    }
                }
                else if (runningProcess.state == 'S' && !SIGUSR1INT)
                {
                    runningProcess.state='R';
                    kill(runningProcess.PID,SIGCONT);
                    prevClk=getClk();
                    runningProcess.waitingTime+=getClk()-runningProcess.lastStopped;
                    printf("resumed process id:%d , PID:%d\n",runningProcess.id,runningProcess.PID);
                    fprintf(outputFile,"At time %d process %d resumed arr %d total %d remain %d wait %d\n",getClk(),runningProcess.id,runningProcess.arrivalTime,runningProcess.executionTime,runningProcess.remaingTime,runningProcess.waitingTime);

                }
                
            }
        }
        //------------------Keeping track of the remaining time of the running process------------------//
        if(prevClk<getClk() && runningProcess.id!=-1)
        {
            runningProcess.remaingTime--;
            prevClk=getClk();
        }
        //------------------Checking whether a premption can occur------------------//
        if (runningProcess.id != -1 && Processes.Head && Processes.Head->process.remaingTime < runningProcess.remaingTime)
        {
            if(canAllocate(Processes.Head->process.memSize) || Processes.Head->process.memStart!=-1){
                fprintf(outputFile,"At time %d process %d stopped arr %d total %d remain %d wait %d\n",getClk(),runningProcess.id,runningProcess.arrivalTime,runningProcess.executionTime,runningProcess.remaingTime,runningProcess.waitingTime);
                
                printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
                runningProcess.state='S';
                runningProcess.lastStopped=getClk();
                kill(runningProcess.PID,SIGUSR2);
                prevClk=getClk();
                priorityEnqueue(&Processes,runningProcess,runningProcess.remaingTime);
                printf("paused process id:%d , PID:%d\n",runningProcess.id,runningProcess.PID);
                runningProcess.id=-1;
            }
            else{
                tempPCBMem=priorityDequeue(&Processes);
                enqueue(&waitingList,*tempPCBMem);
                //free(tempPCBMem);
            }

        }
        

    }
}
void RR(int timeSlice)
{
    struct Queue RR_Queue = {NULL, NULL, 0};
    int RR_TimeSlice = timeSlice;
    struct processData currPD;
    struct msgBuffDummy _buffer = {'c', 0};
    struct PCB _pcb;
    int stat_loc;
    int _pid;
    int _prevCLK;
    int clk;
    int prevCLK = getClk();
    struct PCB *tempPCBMem;
    runningProcess.id = -1;
    SIGUSR1INT = false;
    while (1)
    {

        

        int waitingListCount = waitingList.count;
        if (SIGUSR1INT)
        {
            for (int i = 0; i < waitingListCount; i++)
            {
                tempPCBMem = dequeue(&waitingList);
                if (tempPCBMem && canAllocate(tempPCBMem->memSize))
                {
                    printf("tempPCBMemID=%d ,memSize=%d, i=%d\n", tempPCBMem->id, tempPCBMem->memSize, i);
                    printf("allocated********************\n");
                    allocatedProcessID = tempPCBMem->id;
                    tempPCBMem->memStart = allocate(tempPCBMem->memSize);
                    enqueue(&RR_Queue, *tempPCBMem);
                }
                else
                    enqueue(&waitingList, *tempPCBMem);
                free(tempPCBMem);
            }
            
            SIGUSR1INT = false;
        }
        //------------------Recieving arriving processes from process generator------------------//
        if ((*PG_S_shmaddr).id != prevID)
        {
            currPD = *PG_S_shmaddr;
            if (msgsnd(PG_S_msgqid, &_buffer, sizeof(_buffer), !IPC_NOWAIT) == -1)
                printf("error happened in recv\n");
            _pcb.priority = currPD.priority;
            _pcb.id = currPD.id;
            _pcb.remaingTime = currPD.runningtime;
            _pcb.executionTime = currPD.runningtime;
            _pcb.state = 'W';
            _pcb.arrivalTime = currPD.arrivaltime;
            _pcb.memSize = currPD.memSize;
            _pcb.memStart = -1;
            printf("Arrival time: %d\n", currPD.arrivaltime);
            prevID = currPD.id;
            enqueue(&RR_Queue, _pcb);

            printf("Queue count: %d\n",RR_Queue.count);
        }
        clk = getClk();
        //------------------Starting a new process------------------//
        if (RR_Queue.count != 0)
        {
            if (runningProcess.id == -1)
            {
                runningProcess = *dequeue(&RR_Queue);
                //------------------Checking whether the process has been forked------------------//
                if (runningProcess.state == 'W')
                {
                    //------------------Checking whether the process has already been allocated and whether it can be allocated------------------//
                    if (runningProcess.memStart == -1)
                    {
                        allocatedProcessID = runningProcess.id;
                        runningProcess.memStart = allocate(runningProcess.memSize);
                    }
                    //------------------adding the processes that can't be allocated to the waiting list------------------//
                    if (runningProcess.memStart == -1)
                    {
                        enqueue(&waitingList, runningProcess);
                        runningProcess.id = -1;
                    }
                    else
                    {
                        runningProcess.waitingTime = clk - runningProcess.arrivalTime;
                        int PID = fork();

                        if (PID == 0)
                        {

                            int check1 = execl("./process", (char *)&runningProcess.remaingTime, NULL);
                            if (check1 == -1)
                                printf("Unsuccessful execl with error%d\n", errno);
                        }
                        _prevCLK = clk;
                        runningProcess.PID = PID;
                        runningProcess.state = 'R';
                        printf(" a process started ID= %d , PID = %d , \n", runningProcess.id, runningProcess.PID);
                        fprintf(outputFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", clk, runningProcess.id, runningProcess.arrivalTime, runningProcess.executionTime, runningProcess.remaingTime, runningProcess.waitingTime);
                    }
                }
                else if (runningProcess.state == 'S')
                {
                    runningProcess.state = 'R';
                    kill(runningProcess.PID, SIGCONT);
                    _prevCLK = clk;
                    printf("resumed process id:%d , PID:%d\n", runningProcess.id, runningProcess.PID);
                    fprintf(outputFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", clk, runningProcess.id, runningProcess.arrivalTime, runningProcess.executionTime, runningProcess.remaingTime, runningProcess.waitingTime);
                }
            }
            else
            {
                //------------------Checking whether a premption can occur------------------//
                if (((clk - _prevCLK)%RR_TimeSlice==0)&& ((clk - _prevCLK) !=0) && runningProcess.id != -1)
                {
                    
                    runningProcess.remaingTime -= ((clk - _prevCLK));
                    runningProcess.state = 'S';
                    if (runningProcess.remaingTime > 0)
                    {
                        fprintf(outputFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", clk, runningProcess.id, runningProcess.arrivalTime, runningProcess.executionTime, runningProcess.remaingTime, runningProcess.waitingTime);
                        enqueue(&RR_Queue, runningProcess);
                        runningProcess.id = -1;
                        runningProcess.state = 'S';
                        kill(runningProcess.PID, SIGUSR2);
                    }
                    printf("\nSwitching process..........\n");
                    _prevCLK = getClk();
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    // initClk();
    ProcessQueueInit(&waitingList);
    floatQueueInit(&WTAqueue);
    for(int i=0;i<9;i++)
        InitFloatPriorityQueue(&memQueueArr[i]);
    floatPriorityEnqueue(&memQueueArr[8],0,0);
    floatPriorityEnqueue(&memQueueArr[8],256,256);
    floatPriorityEnqueue(&memQueueArr[8],512,512);
    floatPriorityEnqueue(&memQueueArr[8],768,768);
    memManagementAlgo=(int)(*argv[1])-'0';
    
    signal(SIGUSR1, childHandler);
    signal(SIGINT,exitHandler);
    printf("%c\n", *argv[0]);

    //------------------Creating a shm with Scheduler------------------//
    PG_S_shmid = shmget(SHKEY_PG_S, sizeof(struct processData), IPC_CREAT | 0666);
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


    initClk();
    prevID = -1;
    //------------------Getting the msgq between the process generator and scheduler------------------//
    key_t key_id = ftok("keyfile", MSGKEY_PG_S);
    PG_S_msgqid = msgget(key_id, 0666 | IPC_CREAT);


    //------------------Opening the output file scheduler.log and memory.log------------------//
    outputFile=fopen("scheduler.log","w");
    outputFileMem=fopen("memory.log","w");
    
    if(outputFile==NULL)
        printf("NULL\n");
    
    //------------------Calling the required algorithm------------------//

    if (*argv[0] == '1')
        HPF();
    else if (*argv[0] == '2')
        SRTN();
    else  if (*argv[0] == '3')
    {
        int timeSlice=(int)*argv[2];
        printf("time slice = %d\n",timeSlice);
        RR(timeSlice);
    }
    fclose(outputFile);

}