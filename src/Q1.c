#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>
#include "utils.h"
#include "registers.h"

#define BUFSIZE     256
#define THREADS_MAX 1000

#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

typedef struct bit{
    unsigned x:1;
}bit;

int places[4]; //com o intervalo entre pedidos 10 ms e valor maximo de uso 1000 ms, o valor maximo de lugares em uso
                //ao mesmo tempo é 100. 100/32 =3.125 faz com que seja preciso tamanho 4
bit closed;

int nthreads;
pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mut2=PTHREAD_MUTEX_INITIALIZER; 


void * thread_func(void *arg){
    char * request = (char *) arg;

    int threadi, pid, dur, place;
    long tid;
    sscanf(request,"[ %d, %d, %lu, %d, %d ]",&threadi, &pid, &tid, &dur, &place);
    printRegister(elapsedTime(), threadi, getpid(), tid, dur, -1, RECVD);

    char privateFifo[BUFSIZE]="tmp/";
    char temp[BUFSIZE];
    sprintf(temp,"%d",pid);
    strcat(privateFifo,temp);
    strcat(privateFifo,".");
    sprintf(temp,"%ld",tid);
    strcat(privateFifo,temp);

    int fd_priv;
    do{
        fd_priv = open(privateFifo, O_WRONLY);
    }while(fd_priv==-1);
    if (fd_priv < 0) {
      perror("[Server]Error opening private FIFO"); 
      exit(1);}
    
    pthread_mutex_lock(&mut); 
    int tmp=0;
    while(TestBit(places,tmp)>0){
      tmp++;
    }
    place=tmp;
    SetBit(places, place);
    pthread_mutex_unlock(&mut); 

    if(closed.x){
        place=-1;
        printRegister(elapsedTime(), threadi, getpid(), tid, dur, place, TLATE);
    }
    else
        printRegister(elapsedTime(), threadi, getpid(), tid, dur, place, ENTER);
    
    char sendMessage[BUFSIZE];
    sprintf(sendMessage,"[ %d, %d, %ld, %d, %d ]", threadi, pid, tid, dur, place);
    sleep(1);
    write(fd_priv,&sendMessage,BUFSIZE);
    //printf("-server wrote: %s\n",sendMessage);

    usleep(dur*1000); //espera o tempo de utilizacao do wc

    ClearBit(places, place);

    printRegister(elapsedTime(), threadi, getpid(), tid, dur, place, TIMUP);

    close(fd_priv);
    unlink(privateFifo);
    //printf("server thread returning.......\n");

  return NULL;
}

int main(int argc, char* argv[]) {
    char fifoname[BUFSIZE];
    char fifopath[BUFSIZE]="tmp/";
    double nsecs;
    pthread_t threads[THREADS_MAX];
    int thr=0;
    closed.x=0;
    for (int i = 0; i < 4; i++)
      places[i] = 0;
    
    if (argc!=4) {
        printf("Usage: U1 <-t secs> fifoname\n");
        exit(1);
    }

    //read arguments
    strcpy(fifoname,argv[3]);
    nsecs=atoi(argv[2])*1000;
    strcat(fifopath,fifoname);

    //create public fifo
    if(mkfifo(fifopath,0660)<0){perror("Error creating public FIFO"); exit(1);}
    
    //start counting time
    startTime();
    
    int fd_pub = open(fifopath,O_RDONLY); // sem O_NONBLOCK aqui fica bloqueado à espera que cliente abra
    if (fd_pub==-1){perror("Error opening public FIFO: "); exit(1);}
    
    char clientRequest[BUFSIZE];
    while(elapsedTime() < (double) nsecs){
        //printf("--time elapsed: %f nsecs: %f\n",elapsedTime(),nsecs);
        
        while(read(fd_pub,&clientRequest,BUFSIZE)<=0){ //loop ate encontrar algo p ler
            if (elapsedTime() > (double) nsecs){
                close(fd_pub);
                unlink(fifopath);
                pthread_exit((void*)0);
            }
        }
        //printf("Server created thread\n");
        pthread_create(&threads[thr], NULL, thread_func, &clientRequest);
        pthread_detach(threads[thr]);
        thr++;
    }
    closed.x=1;
    close(fd_pub);
    unlink(fifopath);
    pthread_exit((void*)0);
}