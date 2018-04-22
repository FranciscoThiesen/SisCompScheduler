#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int occupied[60]; // array to see if a second is available for a new real-time process

// vai precisar ter um trecho de memória compartilhada
struct process {
    char* name;
    int type, priority, start, duration;
} typedef struct process proc;


void newRealTime(process p) {
    // checking for any conflict with another realTime process
    if(p.start + p.duration > 60) {
        printf("Erro de overflow na criacao do processo de nome = %s\n", p.name);
        exit(-1);
    }
    for(int i = proc.start; i < proc.start + proc.duration; ++i) {
        if(occupied[i]) {
            printf("Erro, processo de nome %s quer usar segundos ocupados\n", p.name);
            exit(-2);
        }
    }
}

void newPriority(process p) {

}

void newRoundRobin(process p) {

}

int main() {

    while(1) {
        // checar se tem um novo processo na memoria compartilhada ( o interpretador vai mandar um processo a cada segunda...
        if( newProcess ) {
            // extrair as informacoes do processo
            proc newBastard;
            newBastard.name = getName();
            newBastard.type = getType();

            if(newBastard.type == 1) {
                newRoundRobin(newBastard);
            }
            else if(newBastard.type == 2) {
                newBastard.priority = getPriority();
                newPriority(newBastard);
            }
            else if(newBastard.type == 3) {
                newBastard.start = getStart();
                newBastard.duration = getDuration();
                newRealTime(newBastard);
            }
            else {
                printf("Processo de tipo invalido\n");
                exit(-1);
            }
        } 
    }
    return 0; 
}
