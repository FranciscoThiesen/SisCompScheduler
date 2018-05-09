#include <stdio.h>
#include <unistd.h>

int main()
{
    for(int i = 0; i < 10; ++i)
    {
        printf("P2 rodando %d\n", i);
        fflush(stdout);
        sleep(1);
    }
}
