#include "interpretador.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

const int bufferLength = 256;
const int maxInputSize = 30;

int main() {
    FILE* pInput;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    int interpret_mem = shmget(0, sizeof(pid_t), IPC_CREAT | S_IRUSR | S_IRWXU);
    pid_t *interpret_pid = (int*) shmat(interpret_mem, 0, 0);
    *interpret_pid = getpid();

    int scheduler_mem = shmget(1, sizeof(pid_t), IPC_CREAT | S_IRUSR | S_IRWXU);
    pid_t *scheduler_pid = (int*) shmat(scheduler_mem, 0, 0);

    pInput = fopen("exec.txt", "r");

    if(pInput == NULL) {
        printf("Falha na leitura do arquivo de executaveis\n");
        exit(-1);
    }

    while( (read = getline(&line, &len, pInput) ) != -1) {
        char* sentence[4];
        int wordIndex = 0;
        
        char* word = strtok(line, " ");
        while(word != NULL) {
            sentence[wordIndex++] = word; 
            word = strtok(NULL, " ");
        }

        // loop pelas palavras da linha se for necessario
        //for(int i = 0; i < wordIndex; ++i) printf("%s ", sentence[i]);
        
        // Podemos separar o tipo de politica de escalonamento de acordo com 
        // o tamanho da sentenca
        // wordIndex == 2 -> roundRobin, wordIndex == 3 -> Prioridades, == 4 -> real-time
        
        int programNameMem = shmget(2, maxInputSize*sizeof(char), IPC_CREAT | S_IRUSR | S_IRWXU);
        char *programName = (char*) shmat(programNameMem, 0, 0);
        programName = sentence[1];

        if (wordIndex == 3) { // prioridade
            int priorityMem = shmget(4, sizeof(int), IPC_CREAT | S_IRUSR | S_IRWXU);
            int *priority;
            priority = (int*) shmat(priorityMem, 0, 0);

            char* priorityParam = sentence[2];
            *priority = priorityParam[3] - '0';
        }
        else if (wordIndex == 4) { // real-time
            int realtimeMem = shmget(5, 2*sizeof(int), IPC_CREAT | S_IRUSR | S_IRWXU);
            int *data = (int*) shmat(realtimeMem, 0, 0);

            char* start = *(sentence[2] + 2);
            char* duration = *(sentence[3] + 2);
            *data = {atoi(start), atoi(duration)};
        }

        kill(*scheduler_pid, SIGSTOP); // send scheduler a signal indicating a line was read
    }

    fclose(pInput);

    if(line) free(line);

    return 0;
}
