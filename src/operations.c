#include "operations.h"

void initOperations(operations *operations, char *argv[], char *envp[]) {

    operations->dir = malloc( strlen(argv[1]));
    strcpy(operations->dir, argv[0]);

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

void printOperations(operations *operations) {
  printf("Current Dir : %s\n", operations->dir); fflush(stdout);
  printf("IWANT: %d\n", operations->IWANT); fflush(stdout);
  printf("IWANT: %d\n", operations->RECVD); fflush(stdout);
  printf("IWANT: %d\n", operations->ENTER); fflush(stdout);
  printf("IWANT: %d\n", operations->IAMIN); fflush(stdout);
  printf("IWANT: %d\n", operations->TIMUP); fflush(stdout);
  printf("IWANT: %d\n", operations->TLATE); fflush(stdout);
  printf("IWANT: %d\n", operations->CLOSD); fflush(stdout);
  printf("IWANT: %d\n", operations->FAILD); fflush(stdout);
  printf("IWANT: %d\n", operations->GAVUP); fflush(stdout);

  // Enviroment Variables print
  /*
  int i=0;
  while (operations->envip[i] != NULL)
  {
    printf("%s\n", operations->envip[i]);
    i++;
  }*/
 
}