#include <stdio.h>
#include <unistd.h>

int main()
{
    for(int i = 0; i < 10; ++i)
    {
        printf("P1 rodando %d\n", i);
        sleep(1);
    }
}
