#include "headers.h"

void clearResources(int);



int main(int argc, char * argv[])
{
    //signal(SIGINT, clearResources);



    
    // TODO Initialization
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    //------------------for the chosen scheduling algorithm---------------//
    printf("choose a scheduling algorithm:\n");
    printf("1-Highest Priority First\n");
    printf("2-Shortest Remaining Time Next\n");
    printf("3-Round Robin\n");
    char algorithm;
    scanf("%c",&algorithm);
    
    char *algorithmAddress=&algorithm;
    

    //------------------Creating clock and scheduling processes----------------//
    if(fork()==0)
        execv("./scheduler",&algorithmAddress);
    if(fork()==0)
        execv("./clk",argv);

    //------------------Creating a shm with Scheduler------------------//
    int PG_S_shmid = shmget(SHKEY_PG_S, sizeof(struct processData), IPC_CREAT | 0644);
    if ((long)PG_S_shmid == -1)
    {
        perror("Error in creating shm between process generator and sched!\n");
        exit(-1);
    }
    struct processData* PG_S_shmaddr = shmat(PG_S_shmid, (void *)0, 0);
    if ((long)PG_S_shmaddr == -1)
    {
        perror("Error in attaching the shm  between process generator and sched!\n");
        exit(-1);
    }
    (*PG_S_shmaddr).id=-1;
    //------------------processing data from file------------------//
    initClk();
    // To get time use this
    int x = getClk();




    FILE* pFile=fopen("processes.txt","r");
    printf("current time is %d\n", x);
    struct processData process;
    if(pFile==NULL)
        printf("couldn't open file");
    int p;
    while(1)
    {
        fscanf(pFile,"%d",&p);
        process.id=p;
        fscanf(pFile,"%d",&p);
        process.arrivaltime=p;
        fscanf(pFile,"%d",&p);
        process.runningtime=p;
        fscanf(pFile,"%d",&p);
        process.priority=p;
        if(feof(pFile))
            break;
        while(process.arrivaltime!=getClk());
        (*PG_S_shmaddr)=process;

        printf("id=%d , arrival time=%d ,clock=%d\n",process.id,process.arrivaltime,getClk());
    }
    while(1);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    //destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
