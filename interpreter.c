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
#define handle_error(msg)                               \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

const int maxNameSize = 30;

static void chomp(char* s)
{
    while(*s && *s != '\n' && *s != '\r') s++;

    *s = 0;
}

int main()
{
    FILE* pInput;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    
    int interpret_mem = shmget(201, sizeof(pid_t), IPC_CREAT | S_IRUSR | S_IRWXU);
    pid_t* interpret_pid = (int*) shmat(interpret_mem, 0, 0);
    *interpret_pid = getpid();
    
    int scheduler_mem = shmget(202, sizeof(pid_t), IPC_CREAT | S_IRUSR | S_IRWXU);
    pid_t* scheduler_pid = (int*) shmat(scheduler_mem, 0, 0);
    
    int programNameMem = shmget(203, maxNameSize * sizeof(char), IPC_CREAT | S_IRUSR | S_IRWXU);
    char* programName = (char*) shmat(programNameMem, 0, 0);
    
    int paramMem = shmget(204, 3 * sizeof(int), IPC_CREAT | S_IRUSR | S_IRWXU);
    int* params = (int*) shmat(paramMem, 0, 0);
    
    pInput = fopen("exec.txt", "r");
    
    if(pInput == NULL)
    {
        printf("Falha na leitura do arquivo de executaveis\n");
        exit(-1);
    }
    
    while( (read = getline(&line, &len, pInput) ) != -1)
    {
        sleep(1);
        printf("Start reading, interpreter id = %d\n",  *interpret_pid);
        
        char* sentence[4];
        int wordIndex = 0;
        
        char* word = strtok(line, " ");
        while(word != NULL)
        {
            chomp(word);
            sentence[wordIndex++] = word;
            printf(" word = %s");
            word = strtok(NULL, " ");
        }
        
        // loop pelas palavras da linha se for necessario
        // for(int i = 0; i < wordIndex; ++i) printf("%d: %s ", i, sentence[i]);
        
        // Podemos separar o tipo de politica de escalonamento de acordo com
        // o tamanho da sentenca
        // wordIndex == 2 -> roundRobin, wordIndex == 3 -> Prioridades, == 4 -> real-time
        
        //programName = sentence[1];
        strcpy( programName, sentence[1] );
        printf("Vou passar para o scheduler o programa %s\n", programName);
        
        if (wordIndex == 2)
        {
            printf("quero agendar um round-robin = %s\n", programName);
            params[0] = 1;
            // printf("round robin\n");
        }
        else if (wordIndex == 3)
        {
            int priority = sentence[2][3] - '0'; // ex: sentence[1] - PR=7
            params[0] = 2;
            params[1] = priority;
            // printf("priority = %d\n", priority);
        }
        else if (wordIndex == 4)
        {   // real-time
            ////// IMPORTANTE, USUARIO TERA QUE ESCREVER NA FORMA PROGRAMA1 I=10 D=05 ... sempre 2 digitos para nums
            int begin = (sentence[2][2] - '0') * 10 + (sentence[2][3] - '0');
            int length = (sentence[3][2] - '0') * 10 + (sentence[3][3] - '0');
            params[0] = 3;
            params[1] = begin;
            params[2] = length;
            // printf("start: %d, duration: %d\n", params[1], params[2]);
        }
        printf("Wake up scheduler! : %d\n", *scheduler_pid);
        // send scheduler a signal indicating a line was read
        if (kill(*scheduler_pid, SIGUSR1) < 0) {
            printf("Error sending signal\n");
            handle_error("signalfd\n");
        }
        printf("\n");
    }
    
    params[0] = params[1] = params[2] = -1; // indicando que acabou
    
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
