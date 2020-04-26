#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "utils.h"

#define BUFSIZE     256
#define THREADS_MAX 100

int i; //global variable 

void * thread_func(void *arg){
    int fd_pub;
    char request[BUFSIZE];
    char * fifoname = (char *) arg;

    fd_pub = open(fifoname,0660);
    if (fd_pub==-1){perror("Error opening public FIFO: "); exit(1);}

    int useTime = (rand() % 40) + 1; //random useTime between 1 and 40

    sprintf(request,"[%d, %ld, %ld, %d, -1]", i, (int)getpid(), (long int)pthread_self(), useTime);
    if (write(fd_pub, &request, BUFSIZE)<0){perror("Error writing request: "); exit(1);}
    close(fd_pub);

    char privateFifo[BUFSIZE]="/tmp/";
    char temp[BUFSIZE];
    sprintf(temp,"%d",(int)getpid());
    strcat(privateFifo,temp);
    strcat(privateFifo,".");
    sprintf(temp,"%ld",(long int)pthread_self());
    strcat(privateFifo,temp);

   return NULL;
}

int main(int argc, char* argv[]) {
    char fifoname[BUFSIZE];
    double nsecs;
    pthread_t threads[THREADS_MAX];
    int thr=0;

    if (argc!=4) {
        printf("Usage: U1 <-t secs> fifoname\n");
        exit(1);
    }
    
    //read arguments
    strcpy(fifoname,argv[3]);
    nsecs=atoi(argv[2])*1000;

    //start counting time
    startTime();

    srand(time(NULL));

    //ciclo de geracao de pedidos
    while(elapsedTime() < (double) nsecs){
        pthread_create(&threads[thr], NULL, thread_func, fifoname);
        pthread_join(threads[thr],NULL);

        thr++;
        sleep(2);
    }

    return 0;
}
