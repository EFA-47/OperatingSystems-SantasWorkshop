#include "queue.c"
#include <stdint.h> // uint32
#include <stdlib.h> // exit, perror
#include <unistd.h> // usleep
#include <stdio.h> // printf
#include <pthread.h> // pthread_*
#include <sys/time.h>
#ifdef __APPLE__
	#include <dispatch/dispatch.h>
	typedef dispatch_semaphore_t psem_t;
#else
	#include <semaphore.h> // sem_*
	typedef sem_t psem_t;
#endif

int simulationTime = 30;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 30; // frequency of emergency gift requests from New Zealand

pthread_t tid[4] = {0};
int thread_count = 0;
Queue *painting;
Queue *assembly;
Queue *packaging;
Queue *delivery;
Queue *QA;
int giftCount = 0;
int taskCount = 0;
int qaSize = 0;
Gift gifts[1024] = {0};
int giftType;
int r;
int createdGifts = 20;

void* ElfA(void *arg); // the one that can paint
void* ElfB(void *arg); // the one that can assemble
void* Santa(void *arg); 
void* ControlThread(void *arg); // handles printing and queues (up to you)
int pthread_sleep (int seconds);
int main(int argc,char **argv);
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

void InitializeWorkers();
void WaitThreads();
void CreateQueues();
void GiftRequest();
void Log();
Task* queueCheck(Queue* queue);
void enqueueCheck(Queue* queue, Task task);

// pthread sleeper function
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;
    
    pthread_mutex_lock(&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);
    
    //Upon successful completion, a value of zero shall be returned
    return res;
}


int main(int argc,char **argv){
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for(int i=1; i<argc; i++){
        if(!strcmp(argv[i], "-t")) 
        {
            simulationTime = atoi(argv[++i]);
        }
        else if(!strcmp(argv[i], "-s"))  
        {
            seed = atoi(argv[++i]);
        }
    }
    
    srand(seed);

    CreateQueues();
    for(int i=0; i<createdGifts; i++){
        r = rand()%20;
        if(r<20){
            giftType = 5;
            if(r<19){
                giftType = 4;
                if(r<18){
                    giftType = 3;
                    if(r< 14){
                        giftType = 2;
                        if(r<10){
                            giftType = 1;
                        }
                    }
                }
            }
        }
        GiftRequest(); // TODO
    }
    
    InitializeWorkers();
    WaitThreads();

    return 0;
}

void InitializeWorkers(){
    pthread_mutex_init(&myMutex, NULL);
    pthread_create(&tid[thread_count++], NULL, ElfA, NULL);
    pthread_create(&tid[thread_count++], NULL, ElfB, NULL);
    pthread_create(&tid[thread_count++], NULL, Santa, NULL);
    pthread_create(&tid[thread_count++], NULL, ControlThread, NULL);
}

void WaitThreads(){
    for (int i = 0; i < thread_count; ++i)
    {
        pthread_join(tid[i], NULL);
    }
}

void CreateQueues(){
    painting = ConstructQueue(1000);
    assembly = ConstructQueue(1000);
    packaging = ConstructQueue(1000);
    delivery = ConstructQueue(1000);
    QA = ConstructQueue(1000);
}

void GiftRequest(){

    printf("giftType = %d\n",giftType);
    switch (giftType)
    {
    case 1: {
        Gift *giftRequest = (Gift*) malloc(sizeof(Gift));
        giftRequest->GiftID = giftCount;
        giftRequest->GiftType = giftType;

        Task *packagingTask = (Task*) malloc(sizeof(Task));
        packagingTask->GiftID = giftRequest->GiftID;
        packagingTask->TaskID = taskCount++;
        packagingTask->TaskType = 'P';
        packagingTask->completed = 0;

        giftRequest->packagingTask = packagingTask;
        gifts[giftRequest->GiftID] = *giftRequest;
        Enqueue(packaging, *packagingTask);

        giftCount++;
        break;
    }
    case 2: {
        Gift *giftRequest = (Gift*) malloc(sizeof(Gift));
        giftRequest->GiftID = giftCount;
        giftRequest->GiftType = giftType;

        Task *paintingTask = (Task*) malloc(sizeof(Task));
        paintingTask->GiftID = giftRequest->GiftID;
        paintingTask->TaskID = taskCount++;
        paintingTask->TaskType = 'C';
        paintingTask->Responsible = 'A';
        paintingTask->completed = 0;

        giftRequest->paintingTask = paintingTask;
        
        gifts[giftRequest->GiftID] = *giftRequest;
        Enqueue(painting, *paintingTask);

        giftCount++;
        break;
    }
    case 3: {
        Gift *giftRequest = (Gift*) malloc(sizeof(Gift));
        giftRequest->GiftID = giftCount;
        giftRequest->GiftType = giftType;

        Task *assemblyTask = (Task*) malloc(sizeof(Task));
        assemblyTask->GiftID = giftRequest->GiftID;
        assemblyTask->TaskID = taskCount++;
        assemblyTask->TaskType = 'A';
        assemblyTask->Responsible = 'B';
        assemblyTask->completed = 0;

        giftRequest->assemblyTask = assemblyTask;
        
        gifts[giftRequest->GiftID] = *giftRequest;
        Enqueue(assembly, *assemblyTask);
        
        giftCount++;
        break;
    }
    case 4: {
        Gift *giftRequest = (Gift*) malloc(sizeof(Gift));
        giftRequest->GiftID = giftCount;
        giftRequest->GiftType = giftType;

        Task *paintingTask = (Task*) malloc(sizeof(Task));
        paintingTask->GiftID = giftRequest->GiftID;
        paintingTask->TaskID = taskCount++;
        paintingTask->TaskType = 'C';
        paintingTask->Responsible = 'A';
        paintingTask->completed = 0;

        Task *qaTask = (Task*) malloc(sizeof(Task));
        qaTask->GiftID = giftRequest->GiftID;
        qaTask->TaskID = taskCount++;
        qaTask->TaskType = 'Q';
        qaTask->Responsible = 'S';
        qaTask->completed = 0;

        giftRequest->paintingTask = paintingTask;
        giftRequest->qaTask = qaTask;
        
        gifts[giftRequest->GiftID] = *giftRequest;
        Enqueue(painting, *paintingTask);
        Enqueue(QA, *qaTask);

        qaSize++;
        giftCount++;
        break;
    }
    case 5: {
        Gift *giftRequest = (Gift*) malloc(sizeof(Gift));
        giftRequest->GiftID = giftCount;
        giftRequest->GiftType = giftType;

        Task *assemblyTask = (Task*) malloc(sizeof(Task));
        assemblyTask->GiftID = giftRequest->GiftID;
        assemblyTask->TaskID = taskCount++;
        assemblyTask->TaskType = 'A';
        assemblyTask->Responsible = 'B';
        assemblyTask->completed = 0;

        Task *qaTask = (Task*) malloc(sizeof(Task));
        qaTask->GiftID = giftRequest->GiftID;
        qaTask->TaskID = taskCount++;
        qaTask->TaskType = 'Q';
        qaTask->Responsible = 'S';
        qaTask->completed = 0;

        giftRequest->assemblyTask = assemblyTask;
        giftRequest->qaTask = qaTask;
        
        gifts[giftRequest->GiftID] = *giftRequest;
        Enqueue(assembly, *assemblyTask);
        Enqueue(QA, *qaTask);

        qaSize++;
        giftCount++;
        break;
    }
    default:
        break;
    }
}

void* ElfA(void *arg){
    time_t start = time(NULL);
    while(true)
    {
        time_t end = time(NULL) - start;
        if (((double) end) > simulationTime)
        {
            break;
        }

        Task* tk1 = queueCheck(packaging);
        if (tk1->TaskID !=  -1)
        {
            printf("ElfA: %d Starts packaging\n", tk1->TaskID);
            pthread_sleep(1);
            gifts[tk1->GiftID].packagingTask->completed = 1;
            gifts[tk1->GiftID].packagingTask->Responsible = 'A';

            Task *deliveryTask = (Task*) malloc(sizeof(Task));
            deliveryTask->completed = 0;
            deliveryTask->GiftID = tk1->GiftID;
            deliveryTask->TaskType = 'D';
            deliveryTask->TaskID = taskCount++;
            printf("ElfA: %d Creates delivery\n", deliveryTask->TaskID);

            gifts[deliveryTask->GiftID].deliveryTask = deliveryTask;
            enqueueCheck(delivery, *deliveryTask);
            continue;
        }

        Task* tk2 = queueCheck(painting);
        if (tk2->TaskID != -1)
        {
            printf("ElfA: %d Starts painting\n", tk2->TaskID);
            pthread_sleep(3);
            gifts[tk2->GiftID].paintingTask->completed = 1;
            gifts[tk2->GiftID].paintingTask->Responsible = 'A';

            if (gifts[tk2->GiftID].GiftType == 4)
            {
                if (gifts[tk2->GiftID].qaTask->completed == 1)
                {
                    Task *packagingTask = (Task*) malloc(sizeof(Task));
                    packagingTask->completed = 0;
                    packagingTask->GiftID = tk2->GiftID;
                    packagingTask->TaskType = 'P';
                    packagingTask->TaskID = taskCount++;
                    printf("ElfA: %d Creates packaging\n", packagingTask->TaskID);

                    gifts[packagingTask->GiftID].packagingTask = packagingTask;
                    enqueueCheck(packaging, *packagingTask);
                }
            }
            else
            {
                Task *packagingTask = (Task*) malloc(sizeof(Task));
                packagingTask->completed = 0;
                packagingTask->GiftID = tk2->GiftID;
                packagingTask->TaskType = 'P';
                packagingTask->TaskID = taskCount++;
                printf("ElfA: %d Creates packaging\n", packagingTask->TaskID);

                gifts[packagingTask->GiftID].packagingTask = packagingTask;
                enqueueCheck(packaging, *packagingTask);
            }

            continue;
        }
    }
    printf("ElfA finish its work\n");
    return 0;
}

void* ElfB(void *arg){
    time_t start = time(NULL);
    while(true)
    {
        time_t end = time(NULL) - start;
        if (((double) end) > simulationTime)
        {
            break;
        }

        Task* tk1 = queueCheck(packaging);
        if (tk1->TaskID != -1)
        {
            printf("ElfB: %d Starts\n", tk1->TaskID);
            pthread_sleep(1);
            tk1->completed = 1;
            tk1->Responsible = 'B';
            gifts[tk1->GiftID].packagingTask->completed = 1;
            gifts[tk1->GiftID].packagingTask->Responsible = 'B';

            Task *deliveryTask = (Task*) malloc(sizeof(Task));
            deliveryTask->completed = 0;
            deliveryTask->GiftID = tk1->GiftID;
            deliveryTask->TaskType = 'D';
            deliveryTask->TaskID = taskCount++;
            printf("ElfB: %d Creates\n", deliveryTask->TaskID);

            gifts[deliveryTask->GiftID].deliveryTask = deliveryTask;
            enqueueCheck(delivery, *deliveryTask);
            continue;
        }

        Task* tk2 = queueCheck(assembly);
        if (tk2->TaskID != -1)
        {
            printf("ElfB: %d Starts assembly\n", tk2->TaskID);
            pthread_sleep(2);
            tk2->completed = 1;
            tk2->Responsible = 'B';
            gifts[tk2->GiftID].assemblyTask->completed = 1;
            gifts[tk2->GiftID].assemblyTask->Responsible = 'B';

            if (gifts[tk2->GiftID].GiftType == 5)
            {
                if (gifts[tk2->GiftID].qaTask->completed == 1)
                {
                    Task *packagingTask = (Task*) malloc(sizeof(Task));
                    packagingTask->completed = 0;
                    packagingTask->GiftID = tk2->GiftID;
                    packagingTask->TaskType = 'P';
                    packagingTask->TaskID = taskCount++;
                    printf("ElfB: %d Creates packaging\n", packagingTask->TaskID);

                    gifts[packagingTask->GiftID].packagingTask = packagingTask;
                    enqueueCheck(packaging, *packagingTask);
                }
            }
            else
            {
                Task *packagingTask = (Task*) malloc(sizeof(Task));
                packagingTask->completed = 0;
                packagingTask->GiftID = tk2->GiftID;
                packagingTask->TaskType = 'P';
                packagingTask->TaskID = taskCount++;
                printf("ElfB: %d Creates packaging\n", packagingTask->TaskID);

                gifts[packagingTask->GiftID].packagingTask = packagingTask;
                enqueueCheck(packaging, *packagingTask);
            }

            continue;
        }
    }
    printf("ElfB finish its work\n");
    return 0;
}

// manages Santa's tasks
void* Santa(void *arg){
    time_t start = time(NULL);
    while(true)
    {
        time_t end = time(NULL) - start;
        if (((double) end) > simulationTime)
        {
            break;
        }

        if (qaSize < 3)
        {
            Task* tk1 = queueCheck(delivery);
            if (tk1->TaskID != -1)
            {
                printf("Santa: %d Starts delivery\n", tk1->TaskID);
                pthread_sleep(1);
                gifts[tk1->GiftID].deliveryTask->completed = 1;
                gifts[tk1->GiftID].deliveryTask->Responsible = 'S';
                continue;
            }
            
        }
        Task* tk2 = queueCheck(QA);
        qaSize--;
        if (tk2->TaskID != -1)
        {
            printf("Santa yapacak QA buldu\n");
            printf("Santa: %d Starts QA\n", tk2->TaskID);
            pthread_sleep(1);
            gifts[tk2->GiftID].qaTask->completed = 1;
            gifts[tk2->GiftID].qaTask->Responsible = 'S';
            if (gifts[tk2->GiftID].GiftType == 4)
            {
                if (gifts[tk2->GiftID].paintingTask->completed == 1)
                {
                    Task *packagingTask = (Task*) malloc(sizeof(Task));
                    packagingTask->completed = 0;
                    packagingTask->GiftID = tk2->GiftID;
                    packagingTask->TaskType = 'P';
                    packagingTask->TaskID = taskCount++;
                    printf("Santa: %d Creates packaging\n", packagingTask->TaskID);

                    gifts[packagingTask->GiftID].packagingTask = packagingTask;
                    enqueueCheck(packaging, *packagingTask);
                }
            }
            else
            {
                if (gifts[tk2->GiftID].assemblyTask->completed == 1)
                {
                    Task *packagingTask = (Task*) malloc(sizeof(Task));
                    packagingTask->completed = 0;
                    packagingTask->GiftID = tk2->GiftID;
                    packagingTask->TaskType = 'P';
                    packagingTask->TaskID = taskCount++;
                    printf("Santa: %d Creates packaging\n", packagingTask->TaskID);

                    gifts[packagingTask->GiftID].packagingTask = packagingTask;
                    enqueueCheck(packaging, *packagingTask);
                }
            }
        }
    }
    printf("Santa finish its work\n");
    return 0;
}

// the function that controls queues and output
void* ControlThread(void *arg){
    time_t start = time(NULL);
    while(true)
    {
        time_t end = time(NULL) - start;
        if (((double) end) > simulationTime)
        {
            break;
        }
    }
    pthread_sleep(1);
    Log();
    printf("Control finish its work\n");
    return 0;
}

void Log(){
    printf("TaskID\t\tGiftID\t\tGiftType\tTaskType\tTaskArrival\tTT\t\tResponsible\n");
    for (int i=0; i<giftCount; i++)
    {
        if (gifts[i].assemblyTask != NULL)
        {
            printf("%d\t\t%d\t\t%d\t\t%c\t\t%d\t\t%d\t\t%c\n", gifts[i].assemblyTask->TaskID,
                    gifts[i].assemblyTask->GiftID,
                    gifts[i].GiftType,
                    gifts[i].assemblyTask->TaskType,
                    gifts[i].assemblyTask->taskArrival,
                    gifts[i].assemblyTask->TT,
                    gifts[i].assemblyTask->Responsible);
        }
        if (gifts[i].deliveryTask != NULL)
        {
            printf("%d\t\t%d\t\t%d\t\t%c\t\t%d\t\t%d\t\t%c\n", gifts[i].deliveryTask->TaskID,
                    gifts[i].deliveryTask->GiftID,
                    gifts[i].GiftType,
                    gifts[i].deliveryTask->TaskType,
                    gifts[i].deliveryTask->taskArrival,
                    gifts[i].deliveryTask->TT,
                    gifts[i].deliveryTask->Responsible);
        }
        if (gifts[i].packagingTask != NULL)
        {
            printf("%d\t\t%d\t\t%d\t\t%c\t\t%d\t\t%d\t\t%c\n", gifts[i].packagingTask->TaskID,
                    gifts[i].packagingTask->GiftID,
                    gifts[i].GiftType,
                    gifts[i].packagingTask->TaskType,
                    gifts[i].packagingTask->taskArrival,
                    gifts[i].packagingTask->TT,
                    gifts[i].packagingTask->Responsible);
        }
        if (gifts[i].paintingTask != NULL)
        {
            printf("%d\t\t%d\t\t%d\t\t%c\t\t%d\t\t%d\t\t%c\n", gifts[i].paintingTask->TaskID,
                    gifts[i].paintingTask->GiftID,
                    gifts[i].GiftType,
                    gifts[i].paintingTask->TaskType,
                    gifts[i].paintingTask->taskArrival,
                    gifts[i].paintingTask->TT,
                    gifts[i].paintingTask->Responsible);
        }
        if (gifts[i].qaTask != NULL)
        {
            printf("%d\t\t%d\t\t%d\t\t%c\t\t%d\t\t%d\t\t%c\n", gifts[i].qaTask->TaskID,
                    gifts[i].qaTask->GiftID,
                    gifts[i].GiftType,
                    gifts[i].qaTask->TaskType,
                    gifts[i].qaTask->taskArrival,
                    gifts[i].qaTask->TT,
                    gifts[i].qaTask->Responsible);
        }
    }
}

Task* queueCheck(Queue* queue){
    pthread_mutex_lock(&myMutex);
    int res = isEmpty(queue);
    Task* task = (Task*) malloc(sizeof(Task));
    if (!res)
    {
        *task = Dequeue(queue);
    } else 
    {
        task->TaskID = -1;
    }
    pthread_mutex_unlock(&myMutex);
    return task;
}

void enqueueCheck(Queue* queue, Task task){
    pthread_mutex_lock(&myMutex);
    Enqueue(queue, task);
    pthread_mutex_unlock(&myMutex);
}
