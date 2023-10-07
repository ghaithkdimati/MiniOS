#include "headers.h"
int PG_S_shmid;
int PG_S_msgqid;

//------------------Clearing ipc resources------------------//
void clearResources()
{
    //------------------Deleting the shared memory------------------//
    if(-1 == (shmctl(PG_S_shmid, IPC_RMID, NULL)))
    {   
        perror("shmctl");
    }  
    //------------------Deleting the  msgq------------------//      
    if(-1 == (msgctl(PG_S_msgqid, IPC_RMID, NULL)))
    {   
        printf("error in deleting msgq\n");
    } 
    //------------------Sending SIGINT to children processes------------------//
    killpg(getpgrp(),SIGINT);
    exit(0);
}


struct msgBuffDummy
{
    char x[1];
    int mtype;
};




int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);



    
    
    char memManagementAlgo;

    
    char algorithm;
    int timeSlice;


    
    algorithm=argv[3][0];
    if(algorithm=='3')
    {

        timeSlice=atoi(argv[5]);
        memManagementAlgo=argv[7][0];
    }
    else
        memManagementAlgo=argv[5][0];
    
printf("argc=%d\n",argc);
    for(int i=0;i<argc;i++)
    {
        if(!strcmp(argv[i],"-q"))
        {
            timeSlice=atoi(argv[++i]);
            printf("timeSliced=%d\n",timeSlice);
        }
        if(!strcmp(argv[i],"-sch"))
        {
            algorithm=argv[++i][0];
            printf("algorithm=%c\n",algorithm);
        }
        if(!strcmp(argv[i],"-mem"))
        {
            memManagementAlgo=argv[++i][0];
            printf("memManagementAlgo=%c\n",memManagementAlgo);
        }
        
    }

    printf("memManagementAlgo: %c, algorithm=%c\n",memManagementAlgo,algorithm);
    //sleep(5);
    char *algorithmAddress=&algorithm;

    //------------------Creating clock and scheduling processes----------------//
    if(fork()==0)
        execl("./clk",argv[0],NULL);
    initClk();
    if(fork()==0)
    {   
        if(algorithm=='3')
            execl("./scheduler",algorithmAddress,&memManagementAlgo,(char*)&timeSlice,NULL);
        else 
            execl("./scheduler",algorithmAddress,&memManagementAlgo,NULL);
    }

    //------------------Creating a shm with Scheduler------------------//
    PG_S_shmid = shmget(SHKEY_PG_S, sizeof(struct processData), IPC_CREAT | 0666);
    if ((long)PG_S_shmid == -1)
    {
        perror("Error in creating shm between process generator and sched!\n");
        exit(-1);
    }
    struct processData* PG_S_shmaddr = shmat(PG_S_shmid, (void *)0, 0);
    (*PG_S_shmaddr).id=-1;
    if ((long)PG_S_shmaddr == -1)
    {
        perror("Error in attaching the shm  between process generator and sched!\n");
        exit(-1);
    }
    //------------------processing data from file------------------//
    // To get time use this
    int x = getClk();


    //------------------creating a msgq between process gen and sched------------------//
    key_t key_id=ftok("keyfile",MSGKEY_PG_S);
    PG_S_msgqid=msgget(key_id,0666 | IPC_CREAT);



    

    FILE* pFile=fopen(argv[1],"r");
    printf("current time is %d\n", x);
    struct processData process;
    if(pFile==NULL)
        printf("couldn't open file");
    
    struct Queue processesQ;
    ProcessQueueInit(&processesQ);



    struct msgBuffDummy dummy;
    dummy.mtype=0;
    dummy.x[1]='c';
    char  ignore [1024];
    int size=100;
    char *c=(char*)malloc(10);
    int p;
    while(1)
    {
        
        fscanf(pFile,"%s",c);
        //------------------Checking whether the line is commented------------------//
        if(c[0]=='#')
        {
            printf(
                "##############################\n"
            );
            fgets(ignore,sizeof(ignore),pFile);
            if(feof(pFile))
                break;
            continue;
        }
        p=atoi(c);
        process.id=p;
        // fscanf(pFile,"%d",&p);
        // process.id=p;
        fscanf(pFile,"%d",&p);
        process.arrivaltime=p;
        fscanf(pFile,"%d",&p);
        process.runningtime=p;
        fscanf(pFile,"%d",&p);
        process.priority=p;
        fscanf(pFile,"%d",&p);
        process.memSize=p;
        if(feof(pFile))
            break;
        //------------------Waiting until the arrival time of process comes then sending it to the scheduler------------------//
        while(process.arrivaltime!=getClk());
        (*PG_S_shmaddr)=process;
        printf("id=%d , arrival time=%d ,clock=%d\n",process.id,process.arrivaltime,getClk());
        msgrcv(PG_S_msgqid,&dummy,sizeof(dummy),0,!IPC_NOWAIT);
        printf("*PG_S_shmaddr.id=%d\n",(*PG_S_shmaddr).id);
    }
    while(1);
    
}
