#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

const int maxInputSize = 30;
const int maxProcess = 100;
const int numPriorities = 8;
const float quantum = 2;

typedef struct {
    char* name;
    int type, priority, start, duration, finished;
    pid_t procPid;
} Process;

typedef struct {
    Process info[101];
    int head;
    int tail;
    size_t size;
} Queue;

Queue* initQueue() {
    Queue *pQueue = (Queue*) malloc(sizeof (Queue));
    pQueue->head = 0;
    pQueue->tail = -1;
    pQueue->size = 0;
    return pQueue;
}

int isEmpty(Queue* pQueue){ return (pQueue->size == 0); }

int enqueue(Queue* pQueue, Process proc) {
    if(pQueue->size == maxProcess || pQueue == NULL) {
        puts("Queue is full or empty!");
        return -1;
    }
    (pQueue->tail) += 1;
    (pQueue->tail) %= maxProcess;
    (pQueue->info)[pQueue->tail] = proc;
    (pQueue->size) += 1;
    return 1;
}

Process* deque(Queue* pQueue) {
    if(isEmpty(pQueue)) {
        puts("You are trying to deque from an empty queue");
        return NULL;
    }
    
    Process* ret = &((pQueue->info)[pQueue->head]);
    (pQueue->head) += 1;
    (pQueue->head) %= maxProcess;
    (pQueue->size) -= 1;
    return ret;
}

// Queue for the round-robin processes
Queue* roundRobinProc;

// We should have a one queue for every priority [0, 7]
Queue* priorityProc[8]; // taking into account that the priority belongs to the range [0, 7]

// Array of size 60 (one for each second) of time slots occupied by
// realTime processes
int occupiedByRealTime[60];

// Array of size 60 (one for each second) of the starting time
// of each realTime process
Process* realTimeProc[60];

void initRealTimeProcesses() {
    Process realTimeProc[60]; // realTime[i] -> Stores the realTime process that starts on second i ( if there is one)
    Process empty;
    empty.type = -1;
    // inicializando o vetor de realTime com processos "nulos"
    for(int i = 0; i < 60; ++i) {
        realTimeProc[i] = empty;
    }
}

void initProcessesQueues() {
    // priority queues
    for(int i = 0; i < numPriorities; ++i) priorityProc[i] = initQueue();
    
    // round robin queue
    roundRobinProc = initQueue();
    
    // realTime processes
    memset(occupiedByRealTime, 0, sizeof occupiedByRealTime); // initially all time slots are free
    initRealTimeProcesses();
}


/*
 INSERT NEW PROCESS INTO RESPECTIVE QUEUE
 */
void newPriority(Process p) {
    enqueue(priorityProc[p.priority], p);
}

void newRoundRobin(Process p) {
    enqueue(roundRobinProc, p);
}

void newRealTime(Process p) {
    if(p.start + p.duration > 60) {
        printf("Erro de overflow na criacao do processo de nome = %s\n", p.name);
        exit(-1);
    }
    for(int i = p.start; i < p.start + p.duration; ++i) {
        if(occupiedByRealTime[i]) {
            printf("Erro, processo de nome %s quer usar segundos ocupados\n", p.name);
            exit(-2);
        }
    }
    realTimeProc[p.start] = &p;
    for(int i = p.start; i < p.start + p.duration; ++i) occupiedByRealTime[i] = 1;
    // inseri um processo realTime
}

// Get next process to execute
Process* nextProcess() {
    for(int i = 0; i < numPriorities; ++i) {
        if( !isEmpty(priorityProc[i]) )
            return deque(priorityProc[i]);
    }
    if( !isEmpty(roundRobinProc) )
        return deque(roundRobinProc);
    
    return NULL;
}

void newProcessHandler(int signal) 
{
    int paramMem = shmget(204, 3 * sizeof(int), IPC_CREAT | S_IRUSR | S_IRWXU );
    int* params = (int*) shmat(paramMem, 0, 0);
    
    int programNameMem = shmget(203, maxInputSize * sizeof(char), IPC_CREAT | S_IRUSR | S_IRWXU );
    char* programName = (char*) shmat(programNameMem, 0, 0);
    
    printf("Receive process of type %d\n", params[0]);
    
    if(params[0] == -1) return; // condicao de termino
    
    Process p;
    printf("program: %s\n", programName);
    printf("params[0]: %d\n", params[0]);

    strcpy(p.name, programName);
    //p.name = "p1";
    p.type = params[0];
    
    // Initializes process and stops execution in order to obtain pid
    pid_t pid = fork();
    if(pid != 0) {
        p.procPid = pid;
        kill(pid, SIGSTOP);
    } else {
        execve(programName, NULL, NULL);
    }
    
    if(params[0] == 1) {
        newRoundRobin(p);
    }
    else if(params[0] == 2) {
        p.priority = params[1];
        newPriority(p);
    }
    else if(params[0] == 3) {
        p.start = params[1];
        p.duration = params[2];
        newRealTime(p);
    }
    else return;// invalid value params[0]
    
    printf("%s -> program read\n", programName);
    
    shmdt(programName);
    shmdt(params);
}

void scheduler() {
    int scheduler_mem = shmget(202, sizeof(pid_t), IPC_CREAT | S_IRUSR | S_IRWXU );
    pid_t* scheduler_pid = (int*) shmat(scheduler_mem, 0, 0);
    *scheduler_pid = getpid();
    
    // Basic structures have been initialized
    printf("scheduler pid = %d\n", *scheduler_pid);
    
    // register new process handler as callback function
    //
    //sig_t t = signal(SIGUSR1, newProcessHandler);
    //printf("num te falei %d\n", signal(SIGUSR1, newProcessHandler) );
    
    if (signal(SIGUSR1, newProcessHandler) < 0) 
    {
        printf("error registering signal\n");
    }
    printf("registered signal handler\n");
    
    initProcessesQueues();
    
    clock_t stTime, curTime;
    stTime = clock();
    int currentSecond = 0;
    
    Process* curProcess = NULL;
    int result, status; // temporary variables to store process meta-data
    
    // 1 if a realTime process is in execution, 0 otherwise
    int executingRealTimeProcess = 0;
    
    // 1 if a round robin process is in execution, 0 otherwise
    int executingRoundRobinProcess = 0;
    clock_t rrStartTime;
    
    while(1) {
        curTime = clock();
        currentSecond = ((curTime - stTime) / CLOCKS_PER_SEC) % 60;
        
        if (executingRealTimeProcess) {
            result = waitpid(curProcess->procPid, &status, WNOHANG);
            if (result != 0) { // process finished execution
                printf("Process %s finished\n", curProcess->name);
                curProcess->finished = 1;
                executingRealTimeProcess = 0;
            } else {
                // if a real time process is executing we must check if its time
                // is up and kill it if necessary
                if (curProcess->duration + curProcess->start == currentSecond) {
                    kill(curProcess->procPid, SIGSTOP);
                    executingRealTimeProcess = 0;
                }
            }
        }
        
        // new real time process starting at the current second
        // that has not yet finished
        if(realTimeProc[currentSecond] != NULL &&
           realTimeProc[currentSecond]->type != -1 &&
           !realTimeProc[currentSecond]->finished) {
            kill(curProcess->procPid, SIGSTOP); // stop current process
            curProcess = realTimeProc[currentSecond]; // set current process to be realtime process
            kill(curProcess->procPid, SIGCONT); // resume real time process
            executingRealTimeProcess = 1;
        }
        
        // If no real time process is currently in execution
        // check which process to execute next. Next process will
        if (!executingRealTimeProcess) {
            Process* nProcess = nextProcess();
            
            if (nProcess != NULL) { // there is an enqueued process to be executed
                // if a round robin process is executing we must check if its time
                // is up before switching processes in case the next process has
                // equal or less priority (in our case, if it is a round robin process)
                if (executingRoundRobinProcess && !nProcess->priority) {
                    // if the current process has surpassed its quantum, then it should
                    // be switched
                    int clocks_for_quantum = CLOCKS_PER_SEC * quantum;
                    int curProcExecutionTime = (curTime - rrStartTime / clocks_for_quantum );
                    if (curProcExecutionTime >= quantum) {
                        kill(curProcess->procPid, SIGSTOP); // stop current process
                        curProcess = nProcess;
                        kill(curProcess->procPid, SIGCONT); // resume real time process
                    }
                } else {
                    kill(curProcess->procPid, SIGSTOP); // stop current process
                    curProcess = nProcess;
                    kill(curProcess->procPid, SIGCONT); // resume real time process
                }
                
                if (!curProcess->priority) { // save start time for round robin processes
                    executingRoundRobinProcess = 1;
                    rrStartTime = curTime;
                }
                
                result = waitpid(curProcess->procPid, &status, WNOHANG);
                if (result != 0) {
                    printf("Process %s finished\n", curProcess->name);
                    curProcess->finished = 1;
                } else {
                    kill(curProcess->procPid, SIGCONT); // resume process
                }
            }
        }
        if (curProcess != NULL)
            printf("Executing process %s\n", curProcess->name);
        
        // falta tambem indicar que os processos acabaram, para que nosso scheduler eventualmente morra e tal...
        // vamos ter que usar algo do tipo wait(pid_processo, &status, ALGUMA FLAG)...
        //
    }
    
    shmdt(scheduler_pid);
    shmctl(scheduler_mem, IPC_RMID, 0);
}

int main() {
    scheduler();
    return 0;
}
