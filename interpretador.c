#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>

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

    int programNameMem = shmget(2, maxInputSize*sizeof(char), IPC_CREAT | S_IRUSR | S_IRWXU);
    char *programName = (char*) shmat(programNameMem, 0, 0);
        
    int paramMem = shmget(4, 3*sizeof(int), IPC_CREAT | S_IRUSR | S_IRWXU);
    int *params = (int*) shmat(paramMem, 0, 0);

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
         
        programName = sentence[0];
        printf("%s\n", programName);
        if (wordIndex == 1) {
            //params = { 2 }
            params[0] = 2;
            // printf("%d\n", params[0]);
        }
        else if (wordIndex == 2) { // prioridade
            printf("prioridade = %c \n", sentence[1][3]);
            int priority = sentence[1][3] - '0';
            //params = { 3, priority }
            params[0] = 3;
            params[1] = priority;
            // printf("%d %d\n", params[0], params[1]);
        }
        else if (wordIndex == 3) { // real-time
            ////// IMPORTANTE, USUARIO TERA QUE ESCREVER NA FORMA PROGRAMA1 I=10 D=05 ... sempre 2 digitos para nums
            int begin = (sentence[1][2] - '0') * 10 + (sentence[1][3] - '0');
            int length = (sentence[2][2] - '0') * 10 + (sentence[2][3] - '0');            
            //params = { 4, atoi(start), atoi(duration)};
            params[0] = 4;
            params[1] = begin;
            params[2] = length;
            
            // printf("%d %d %d\n", params[0], params[1], params[2]);
        }
        
        //kill(*scheduler_pid, SIGSTOP); // send scheduler a signal indicating a line was read
    }

    shmdt(interpret_pid);
    shmdt(scheduler_pid);
    shmdt(programName);
    shmdt(params);

    shmctl(interpret_mem, IPC_RMID, 0);
    shmctl(scheduler_mem, IPC_RMID, 0);
    shmctl(programNameMem, IPC_RMID, 0);
    shmctl(paramMem, IPC_RMID, 0);

    fclose(pInput);

    if(line) free(line);

    return 0;
}
