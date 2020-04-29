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
int nthreads;
pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; 

void * thread_func(void *arg){
    pthread_mutex_lock(&mut); 
    nthreads++;
    pthread_mutex_unlock(&mut);

    //printf("Client created thread\n");
    int fd_pub;
    char request[BUFSIZE];
    char * fifopath = (char *) arg;

    int useTime = (rand() % 1000) + 1; //random useTime between 1 and 200

    sprintf(request,"[ %d, %d, %ld, %d, -1 ]", i, (int)getpid(), (long int)pthread_self(), useTime);

    //printf("|Client opening public fifo : %s aaaa\n", fifopath);
    fd_pub = open(fifopath,O_WRONLY);
    if (fd_pub==-1){
        //error("Error opening public FIFO: ");
        printRegister(elapsedTime(), i, (int)getpid(), (long int)pthread_self(), useTime, -1, CLOSD);
        pthread_exit((void*)1);
    }

    printRegister(elapsedTime(), i, (int)getpid(), (long int)pthread_self(), useTime, -1, IWANT);
    
    if (write(fd_pub, &request, BUFSIZE)<0){
        perror("Error writing request: "); exit(1);}
    
    close(fd_pub);
    
    //printf("-client wrote: %s\n",request);

    char privateFifo[BUFSIZE]="tmp/";
    char temp[BUFSIZE];
    sprintf(temp,"%d",(int)getpid());
    strcat(privateFifo,temp);
    strcat(privateFifo,".");
    sprintf(temp,"%ld",(long int)pthread_self());
    strcat(privateFifo,temp);

    //printf("--U:creatingPrivateFifo %s\n", privateFifo);
    //create private fifo to read message from server
    if(mkfifo(privateFifo,0660)<0){perror("Error creating private FIFO:"); exit(1);}

    //printf("--U:openingPrivateFifo %s\n", privateFifo);
    int fd_priv = open(privateFifo, O_RDONLY);
    if (fd_priv < 0) {perror("[Client]Error opening private FIFO: "); exit(1);}

    char receivedMessage[BUFSIZE];

    if(read(fd_priv,&receivedMessage,BUFSIZE)<=0) {
        printRegister(elapsedTime(), i, (int)getpid(), (long int)pthread_self(), useTime, -1, FAILD);
    }

    //printf("-client received: %s\n",request);
    int threadi, pid, dur, place;
    long int tid;
    sscanf(request,"[ %d, %d, %ld, %d, %d ]",&threadi, &pid, &tid, &dur, &place);

    printRegister(elapsedTime(), i, (int)getpid(), (long int)pthread_self(), useTime, -1, IAMIN);
    
    close(fd_priv);
    unlink(privateFifo);
    //printf("client returning.......\n");

  return NULL;
}

int main(int argc, char* argv[], char *envp[]) {
    nthreads = 0;
    char fifoname[BUFSIZE];
    char fifopath[BUFSIZE]="tmp/";
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
    strcat(fifopath,fifoname);

    /*printf("argv[0]: %s\n", argv[0]);
    printf("argv[1]: %s\n", argv[1]);
    printf("argv[2]: %s\n", argv[2]);
    printf("argv[3]: %s\n", argv[3]);

    printf("Register format:\n");
    printRegister(0.5, 23, 132, 135, 20000, 10, IWANT);*/

    //start counting time
    startTime();

    srand(time(NULL));

    //ciclo de geracao de pedidos
    while(elapsedTime() < (double) nsecs){
        pthread_create(&threads[thr],NULL, thread_func, (void *)fifopath);
        pthread_detach(threads[thr]);
        thr++;
        usleep(50*1000);
    }
    
    //printf("Client exiting\n");
    printf("CLIENT - NTHREAD -> %d\n", nthreads);
    pthread_exit((void*)0);
}
