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

#define BUFSIZE     256    /**< nº of bytes written and read between fifos*/
//#define THREADS_MAX 2000   /**< max number of threads */

#define MSATTEMPT 500 /**< nº of milisec to waste attempting to open private fifo*/

#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )

//com o intervalo entre pedidos 10 ms e valor maximo de uso 1000 ms, o valor maximo de lugares em uso
//ao mesmo tempo é 100. 100/32 =3.125 faz com que seja preciso tamanho 4
int places[4];  /**< array of bits to store used places */
bit closed; /**< bit to store if Server is open or closed */

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; /**< mutex to access places[] */
pthread_mutex_t mut2=PTHREAD_MUTEX_INITIALIZER; /**< mutex to enable closed bit */

double nsecs;  /**< numbers of seconds the program will be running */

void * thread_func(void *arg){
    char * request = (char *) arg; /**< Request string received from public fifo*/
    int threadi, pid, dur, place;  /**< Component of request*/
    long tid;  /**< thread id of client */

    // parse contents received from public fifo
    if(sscanf(request,"[ %d, %d, %lu, %d, %d ]",&threadi, &pid, &tid, &dur, &place)==EOF){perror("Server-sscanf");}
    printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, -1, RECVD);

    // make private fifo pathname
    char privateFifo[BUFSIZE]="/tmp/";
    char temp[BUFSIZE];
    if(sprintf(temp,"%d",pid)<0){perror("Server-sprintf");}
    strcat(privateFifo,temp);
    strcat(privateFifo,".");
    if(sprintf(temp,"%ld",tid)<0){perror("Server-sprintf");}
    strcat(privateFifo,temp);

    // open private fifo
    int fd_priv; /**< private fifo file descriptor */
    float startt = elapsedTime();
    do{
        fd_priv = open(privateFifo, O_WRONLY);
    } while (fd_priv == -1 && elapsedTime() - startt < MSATTEMPT);
  
    if (fd_priv < 0){
        fprintf(stderr, "%d-%s\n", threadi, "Server - Error Opening Private Fifo");
        if(close(fd_priv)==-1) {perror("Error closing fifo:");}
        pthread_exit((void *)1);
    }
    
    // Finding available place
    int tmp=0;
    if(pthread_mutex_lock(&mut)!=0){perror("Server-MutexLock");}
    while(TestBit(places,tmp)){tmp++;}
    place=tmp;
    SetBit(places, place);
    if(pthread_mutex_unlock(&mut)!=0){perror("Server-MutexUnLock");}

    // sending message with place to private fifo
    char sendMessage[BUFSIZE];  /**< string with message to send */
    
    // checking if server is closed
  if(pthread_mutex_lock(&mut2)!=0){perror("Server-MutexLock");}
    if(closed.x){
        if(pthread_mutex_unlock(&mut2)!=0){perror("Server-MutexUnLock");}
        place=-1;
        dur = -1;
        printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, place, TLATE);
    }
    else{
        if(pthread_mutex_unlock(&mut2)!=0){perror("Server-MutexUnLock");}
        printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, place, ENTER);
    }
    if(sprintf(sendMessage,"[ %d, %d, %ld, %d, %d ]", threadi, getpid(), pthread_self(), dur, place)<0){perror("Server-sprintf");}

    // write answer to private fifo
    if(write(fd_priv,&sendMessage,BUFSIZE) == -1){
      printRegister(time(NULL), threadi, pid, pthread_self(), dur, place, GAVUP);
      pthread_exit((void *)1);
    }

    if(closed.x){ //nao espera o tempo de uso
        if(close(fd_priv)==-1){perror("Server-closePrivateFifo");}
        pthread_exit(NULL);
    }

    // wait using time
    if(usleep(dur*1000) == -1){perror("Server-usleep");}
    printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, place, TIMUP);
    
    // cleanup
    ClearBit(places, place);
    if(close(fd_priv)==-1){perror("Server-closePrivateFifo");}
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    char fifoname[BUFSIZE];  /**< public fifo file name */
    char fifopath[BUFSIZE]="/tmp/";  /**< public fifo path */
    char clientRequest[BUFSIZE];  /**< string read from public fifo */
    pthread_t tid;  /**< array to store thread id's */
    closed.x=0;  /**< 1-server closed | 0-server open */

    // initialize available places at 0
    for (int i = 0; i < 4; i++)
      places[i] = 0;
    
    // check arguments
    if (argc!=4) {
        printf("Usage: U1 <-t secs> fifoname\n");
        exit(1);
    }

    //read arguments
    strcpy(fifoname,argv[3]);
    nsecs=atoi(argv[2])*1000;
    strcat(fifopath,fifoname);

    //create public fifo
    if(mkfifo(fifopath,0660)==-1){perror("Error creating public FIFO"); exit(1);}
    
    //start counting time
    startTime();
    
    // open public fifo
    int fd_pub = open(fifopath,O_RDONLY); // sem O_NONBLOCK aqui fica bloqueado à espera que cliente abra
    if (fd_pub==-1){perror("Error opening public FIFO: "); exit(1);}
    
    // while loop to check running time
    while(elapsedTime() < (double) nsecs){        
        // while loop to check public fifo
        if(read(fd_pub,&clientRequest,BUFSIZE)<=0){ continue;}
        // create thread with contents of public fifo
        if(pthread_create(&tid, NULL, thread_func, &clientRequest)!=0){perror("Server-pthread_Create");}
        if(pthread_detach(tid)!=0){perror("Server-pthread_detach");}
    }
    // closing sequence
    if(unlink(fifopath)==-1){perror("Error destroying public fifo:");}
    if(pthread_mutex_lock(&mut2)!=0){perror("Server-MutexLock");}
    closed.x = 1;
    if(pthread_mutex_unlock(&mut2)!=0){perror("Server-MutexUnLock");}
    //float starttime;
    int readreturn = read(fd_pub, &clientRequest, BUFSIZE);
    while (readreturn != 0)
    {
      printf("Dentro de while\n");
      if(readreturn <0 ){perror("Server-read error");}
      if(pthread_create(&tid, NULL, thread_func, &clientRequest)!=0){perror("Server-pthread_Create");}
      if(pthread_detach(tid)!=0){perror("Server-pthread_detach");}
      readreturn = read(fd_pub, &clientRequest, BUFSIZE);
    }
    
    // cleanup
    if(close(fd_pub)==-1){perror("Server-closePublicFifo");}
    pthread_exit((void*)0);
}
