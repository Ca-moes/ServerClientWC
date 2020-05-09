#include <pthread.h>
#include <stdio.h>
#include <math.h>
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

double nsecs;  /**< numbers of seconds the program will be running */
int nplaces; /**< size of array places */
int nThreadsActive; /**< nº of active threads at the moment*/

//com o intervalo entre pedidos 10 ms e valor maximo de uso 1000 ms, o valor maximo de lugares em uso
//ao mesmo tempo é 100. 100/32 =3.125 faz com que seja preciso tamanho 4
int *places;  /**< array of bits to store used places */
bit closed; /**< bit to store if Server is open or closed */


pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; /**< mutex to access places[] */
pthread_mutex_t mut2=PTHREAD_MUTEX_INITIALIZER; /**< mutex to enable closed bit */
pthread_mutex_t mut3=PTHREAD_MUTEX_INITIALIZER; /**< mutex to access nThreadsActive */



void * thread_func(void *arg){
    if(pthread_mutex_lock(&mut3)!=0){perror("Server-MutexLock");}
    nThreadsActive++;
    if(pthread_mutex_unlock(&mut3)!=0){perror("Server-MutexUnLock");}

    char * request = (char *) arg; /**< Request string received from public fifo*/
    int threadi, pid, dur, place;  /**< Component of request*/
    long tid;  /**< thread id of client */

    // parse contents received from public fifo
    if(sscanf(request,"[ %d, %d, %lu, %d, %d ]",&threadi, &pid, &tid, &dur, &place)==EOF){perror("Server-sscanf");}
    printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, -1, RECVD);

    // make private fifo pathname
    char privateFifo[BUFSIZE]="tmp/";
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
    if(pthread_mutex_lock(&mut3)!=0){perror("Server-MutexLock");}
    nThreadsActive--;
    if(pthread_mutex_unlock(&mut3)!=0){perror("Server-MutexUnLock");}
    pthread_exit(NULL);
}

void argumentsReader(int argc, char* argv[], int *nplaces, int *nthreads, char fifoname[]){
  /* 1 - ler valor de -t
   * 2 - ler valor de -l
   * 3 - ler valor de -n */
  int flag = 0;
  for (int i = 1; i < argc; i++)
  {
    if (flag == 1)
    {
      nsecs=atoi(argv[i])*1000;
      flag = 0;
      continue;
    }
    else if (flag == 2)
    {
      *nplaces = atoi(argv[i]);
      flag = 0;
      continue;
    }
    else if (flag == 3)
    {
      *nthreads = atoi(argv[i]);
      flag = 0;
      continue;
    }
    else if (strcmp(argv[i], "-t") == 0)
    {
      flag = 1;
      continue;
    }
    else if (strcmp(argv[i], "-l") == 0)
    {
      flag = 2;
      continue;
    }
    else if (strcmp(argv[i], "-n") == 0)
    {
      flag = 3;
      continue;
    }
    else
      strcpy(fifoname,argv[i]);
  }
}

int main(int argc, char* argv[]) {
    char fifoname[BUFSIZE];  /**< public fifo file name */
    char fifopath[BUFSIZE]="tmp/";  /**< public fifo path */
    char clientRequest[BUFSIZE];  /**< string read from public fifo */
    pthread_t tid;  /**< array to store thread id's */
    closed.x=0;  /**< 1-server closed | 0-server open */
    nplaces = 0;
    nThreadsActive = 0;
    int nthreads=INT_MAX;
    // check arguments
    if (argc!=4 && argc!=6 && argc!=8){
        printf("Usage: U1 <-t secs> fifoname\n");
        exit(1);
    }

    //read arguments
    argumentsReader(argc, argv, &nplaces, &nthreads, fifoname);
    //printf("argc:%d\nargv:smth\nnsecs:%f\nnplaces:%d\nnthreads:%d\nfifoname:%s\n", argc, nsecs, nplaces, nthreads, fifoname);
    strcat(fifopath,fifoname);
    int sizearr = (int)ceil(nplaces/32.0);
    places = (int*) malloc(sizearr * sizeof(int));
    int* places_copy = places;

    // initialize available places at 0
    for (int i = 0; i < sizearr; i++)
      places[i] = 0;

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
        if (nThreadsActive < nthreads)
        {
          if(pthread_create(&tid, NULL, thread_func, &clientRequest)!=0){perror("Server-pthread_Create");}
          if(pthread_detach(tid)!=0){perror("Server-pthread_detach");}
        }
        else
        {
          // o que é suposto fazer aqui?
        }
        
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
    free(places_copy);
    pthread_exit((void*)0);
}
