#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


int occupied[60]; // array to see if a second is available for a new real-time process

// vai precisar ter um trecho de memória compartilhada
struct process{
    char* name;
    int type, priority, start, duration;
}typedef struct process proc;


void newRealTime(process p)
{
    
}

void newPriority(process p)
{

}

void newRoundRobin(process p)
{

}

int main()
{
    while(1)  
    {
        // checar se tem um novo processo na memoria compartilhada ( o interpretador vai mandar um processo a cada segunda...
        if( newProcess )
        {
            // extrair as informacoes do processo
            proc nuevo;
            nuevo.name = getName();
            nuevo.type = getType();

            if(nuevo.type == 1) // RoundRobin Process
            {
                newRoundRobin(nuevo);
            }
            else if(nuevo.type == 2) // priority process
            {
                nuevo.priority = getPriority();
                newPriority(nuevo);
            }
            else if(nuevo.type == 3)
            {
                nuevo.start = getStart();
                nuevo.duration = getDuration();
                newRealTime(nuevo);
            }
            else
            {
                printf("Processo de tipo invalido\n");
                exit(-1);
            }
        } 
    }
    return 0; 
}
