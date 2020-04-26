#include "operations.h"

void initOperations(operations *operations, char *envp[]) {
    int i=0, j=0;
    while (envp[i] != 0)
        i++;
    operations->envip = malloc( sizeof(char*) * i+1 ); //+1 para o NULL
  
    while (j<i)
    {
        operations->envip[j] = envp[j];
        j++;
    }
    operations->envip[j] = NULL;

    operations->IWANT = 0;
    operations->RECVD = 0;
    operations->ENTER = 0;
    operations->IAMIN = 0;
    operations->TIMUP = 0;
    operations->TLATE = 0;
    operations->CLOSD = 0;
    operations->FAILD = 0;
    operations->GAVUP = 0;
}