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

// Limit Values for Random user usage times
#define UPPERB 1000
#define LOWERB 1
// Interval betweeen User Requests (miliseconds)
#define INTMS 50

int i; //global variable 
pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; 

void * thread_func(void *arg){
    pthread_mutex_lock(&mut); 
    i++;
    pthread_mutex_unlock(&mut);

    //printf("Client created thread\n");
    int fd_pub;
    char request[BUFSIZE];
    char * fifopath = (char *) arg;

    int useTime = (rand() % (UPPERB - LOWERB + 1)) + LOWERB;  //random useTime between 1 and 100

    fd_pub = open(fifopath,O_WRONLY);
    if (fd_pub==-1){
        printRegister(elapsedTime(), i, (int)getpid(), (long int)pthread_self(), useTime, -1, CLOSD);
        pthread_exit((void*)1);
    }

    sprintf(request,"[ %d, %d, %lu, %d, -1 ]", i, getpid(), pthread_self(), useTime);
    
    if (write(fd_pub, &request, BUFSIZE)<0){perror("Error writing request: "); exit(1);}

    printRegister(elapsedTime(), i, getpid(), pthread_self(), useTime, -1, IWANT);
    close(fd_pub);
    
    char privateFifo[BUFSIZE]="tmp/";
    char temp[BUFSIZE];
    sprintf(temp,"%d",(int)getpid());
    strcat(privateFifo,temp);
    strcat(privateFifo,".");
    sprintf(temp,"%ld",(long int)pthread_self());
    strcat(privateFifo,temp);

    //create private fifo to read message from server
    if(mkfifo(privateFifo,0660)<0){perror("Error creating private FIFO:"); exit(1);}

    int fd_priv = open(privateFifo, O_RDONLY);
    if (fd_priv < 0) {perror("[Client]Error opening private FIFO: "); exit(1);}

    char receivedMessage[BUFSIZE];
    int tmpresult = read(fd_priv,&receivedMessage,BUFSIZE);

    while(tmpresult<=0){
      // lê de fifo privado continuamente enquanto não há info
      tmpresult = read(fd_priv,&receivedMessage,BUFSIZE);
    }
    if(tmpresult<0) {
      printRegister(elapsedTime(), i, (int)getpid(), (long int)pthread_self(), useTime, -1, FAILD);
    }
    int threadi, pid, dur, place;
    long int tid;
    sscanf(receivedMessage,"[ %d, %d, %ld, %d, %d ]",&threadi, &pid, &tid, &dur, &place);
    if (place >= 0)
      printRegister(elapsedTime(), threadi, getpid(), tid, dur, place, IAMIN);
    else
      printRegister(elapsedTime(), threadi, getpid(), tid, dur, place, CLOSD);
    
    

    close(fd_priv);
    unlink(privateFifo);
    //printf("client returning.......\n");

  return NULL;
}

int main(int argc, char* argv[], char *envp[]) {
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
        usleep(INTMS*1000);
    }
    
    //printf("Client exiting\n");
    pthread_exit((void*)0);
}
