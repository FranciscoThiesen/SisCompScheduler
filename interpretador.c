#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

const int bufferLength = 256;
const int maxWordSize = 30;

int main()
{
    FILE* pInput;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    pInput = fopen("exec.txt", "r");

    if(pInput == NULL)
    {
        printf("Falha na leitura do arquivo de executaveis\n");
        exit(-1);
    }

    while( (read = getline(&line, &len, pInput) ) != -1)
    {
        char* sentence[4];
        int wordIndex = 0;
        
        char* word = strtok(line, " ");
        while(word != NULL)
        {
            sentence[wordIndex++] = word; 
            word = strtok(NULL, " ");
        }

        // loop pelas palavras da linha se for necessario
        //for(int i = 0; i < wordIndex; ++i) printf("%s ", sentence[i]);
        
        // Podemos separar o tipo de politica de escalonamento de acordo com 
        // o tamanho da sentenca
        // wordIndex == 2 -> roundRobin, wordIndex == 3 -> Prioridades, == 4 -> real-time
        
        if(wordIndex == 2) // roundRobin
        {

        }
        else if(wordIndex == 3) // prioridade
        {

        }
        else // real-time
        {

        }
    }

    fclose(pInput);

    if(line) free(line);

    return 0;
}
