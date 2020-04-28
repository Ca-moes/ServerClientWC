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
#include "registers.h"

#define BUFSIZE     256
#define THREADS_MAX 100

int i; //global variable 

void * thread_func(void *arg){
    int fd_pub;
    char request[BUFSIZE];
    char * fifopath = (char *) arg;

    fd_pub = open(fifopath,O_WRONLY);
    if (fd_pub==-1){perror("Error opening public FIFO: "); exit(1);}

    int useTime = (rand() % 40) + 1; //random useTime between 1 and 40

    sprintf(request,"[%d, %d, %ld, %d, -1]", i, (int)getpid(), (long int)pthread_self(), useTime);
    if (write(fd_pub, &request, BUFSIZE)<0){perror("Error writing request: "); exit(1);}
    close(fd_pub);
    printf("client writed: %s\n",request);

    char privateFifo[BUFSIZE]="/tmp/";
    char temp[BUFSIZE];
    sprintf(temp,"%d",(int)getpid());
    strcat(privateFifo,temp);
    strcat(privateFifo,".");
    sprintf(temp,"%ld",(long int)pthread_self());
    strcat(privateFifo,temp);

    /*
    //create private fifo to read message from server
    if(mkfifo(privateFifo,0660)<0){perror("Error creating private FIFO:"); exit(1);}

    int fd_priv = open(privateFifo, O_RDONLY);
    if (fd_priv < 0) {perror("Error opening private FIFO: "); exit(1);}

    char receivedMessage[BUFSIZE];

    if(read(fd_priv,&receivedMessage,BUFSIZE)<0){perror("Error reading msg from server: "); exit(1);}

    int threadi, pid, dur, place;
    long int tid;
    sscanf(request,"[%d, %d, %ld, %d, %d]",&threadi, &pid, &tid, &dur, &place);
    
    close(fd_priv);
    unlink(privateFifo);
    */

  return NULL;
}

int main(int argc, char* argv[], char *envp[]) {
    char fifoname[BUFSIZE];
    char fifopath[BUFSIZE]="/tmp/";
    double nsecs;
    pthread_t threads[THREADS_MAX];
    int thr=0;

    printf("Uhere0\n");

    if (argc!=4) {
        printf("Usage: U1 <-t secs> fifoname\n");
        exit(1);
    }
    
    printf("Uhere1\n");
    //read arguments
    strcpy(fifoname,argv[3]);
    nsecs=atoi(argv[2])*1000;
    strcat(fifopath,fifoname);
    printf("Uhere2\n");

    /*printf("argv[0]: %s\n", argv[0]);
    printf("argv[1]: %s\n", argv[1]);
    printf("argv[2]: %s\n", argv[2]);
    printf("argv[3]: %s\n", argv[3]);

    printf("Register format:\n");
    printRegister(0.5, 23, 132, 135, 20000, 10, IWANT);*/

    //start counting time
    printf("Uhere3\n");
    startTime();
    printf("Uhere4\n");

    srand(time(NULL));

    printf("Uhere5\n");
    //ciclo de geracao de pedidos
    while(elapsedTime() < (double) nsecs){
        printf("Client created thread\n");
        pthread_create(&threads[thr], NULL, thread_func, fifopath);
        pthread_join(threads[thr],NULL);
        thr++;
        sleep(2);
    }
    
    printf("Client exiting\n");
    return 0;
}
