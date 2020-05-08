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
//#define THREADS_MAX 2000

// Limit Values for Random user usage times
#define UPPERB 1000
#define LOWERB 1
// Interval betweeen User Requests (miliseconds)
#define INTMS 50

#define MSATTEMPT 500 /**< nº of milisec to waste attempting to open public fifo*/

//global variables
int i;           /**< número sequencial do pedido */
bit serverOpen;  /**< 1 if server is open | 0 otherwise*/

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; /**<  mutex para aceder a serverOpen*/
pthread_mutex_t mut2=PTHREAD_MUTEX_INITIALIZER; /**<  mutex para aceder a i*/

/**
 * Thread Function that creates requests
 */
void * thread_func(void *arg){
  // updating i with mutex's
  if(pthread_mutex_lock(&mut2)!=0){perror("Client-MutexLock");}
  i++;
  if(pthread_mutex_unlock(&mut2)!=0){perror("Client-MutexUnLock");}

  int fd_pub; /**< file descriptor of public fifo */
  char request[BUFSIZE]; /**< Request string to send to public fifo*/
  char * fifopath = (char *) arg; /**< public fifo path*/
  int useTime = (rand() % (UPPERB - LOWERB + 1)) + LOWERB; /**< Determine the use time for this client */

  // opening Public fifo to be able to write
  float startt = elapsedTime();
  int attempt = 0;
  do{     
    fd_pub = open(fifopath,O_WRONLY, O_NONBLOCK);
    attempt++;
  } while (fd_pub==-1 && attempt < 5);
  if (fd_pub == -1) {
    if(pthread_mutex_lock(&mut2)!=0){perror("Client-MutexLock");}
    printRegister(time(NULL), i, getpid(), pthread_self(), useTime, -1,  CLOSD);
    if(pthread_mutex_unlock(&mut2)!=0){perror("Client-MutexUnLock");}
    if(pthread_mutex_lock(&mut)!=0){perror("Client-MutexLock");}
    serverOpen.x = 0;
    if(pthread_mutex_unlock(&mut)!=0){perror("Client-MutexUnLock");}
    pthread_exit(NULL);
  }

  // Making of message to send, writing of message and closing of fifo
  if(sprintf(request,"[ %d, %d, %lu, %d, -1 ]", i, getpid(), pthread_self(), useTime)<0){perror("Client-sprintf");}
  
  if (write(fd_pub, &request, BUFSIZE)<0){
      perror("Error writing request: ");
      if(close(fd_pub)==-1){perror("Client-closePublicFifo");} 
      pthread_exit(NULL);
  }

  printRegister(time(NULL), i, getpid(), pthread_self(), useTime, -1, IWANT);
  if(close(fd_pub)==-1){perror("Client-closePublicFifo");}
  
  // Making of pathname of private fifo 
  char privateFifo[BUFSIZE]="tmp/";
  char temp[BUFSIZE];
  if(sprintf(temp,"%d",(int)getpid())<0){perror("Client-sprintf");}
  strcat(privateFifo,temp);
  strcat(privateFifo,".");
  if(sprintf(temp,"%ld",(long int)pthread_self())<0){perror("Client-sprintf");}
  strcat(privateFifo,temp);

  //Create private fifo to read message from server
  if(mkfifo(privateFifo,0660)==-1){perror("Error creating private FIFO:"); pthread_exit(NULL);}

  // Opening private fifo to read the response
  int fd_priv;
  startt = elapsedTime();
  do{
    fd_priv = open(privateFifo, O_RDONLY | O_NONBLOCK);
  } while (fd_priv==-1 && elapsedTime() - startt < MSATTEMPT);
  if (fd_priv < 0) {
    if(fprintf(stderr, "%d.%s\n", i, "Client - Error Opening Private Fifo")<0){perror("Client-fprintf");}
    if(close(fd_priv)==-1){perror("Client-closePrivateFifo");}
    pthread_exit(NULL);
  }

  // Attempting to read the response
  char receivedMessage[BUFSIZE];
  int tmpresult = read(fd_priv,&receivedMessage,BUFSIZE);

  // Attempts to read from private fifo until there's a response
  int try = 0;
  while(tmpresult<=0 && try < 5){
    if (try != 0)
      usleep(100*1000);    
    tmpresult = read(fd_priv,&receivedMessage,BUFSIZE);
    try++;
  }
  if(tmpresult<=0) {
    printRegister(time(NULL), i, getpid(), pthread_self(), useTime, -1, FAILD);
    if(close(fd_priv)==-1){perror("Client-closePrivateFifo");}
    if (unlink(privateFifo)==-1){perror("Error destroying private fifo:");}
    pthread_exit(0);
  }

  // If there's a response, parse the response to different variables
  int threadi, pid, dur, place;
  long int tid;
  if(sscanf(receivedMessage,"[ %d, %d, %ld, %d, %d ]",&threadi, &pid, &tid, &dur, &place)==EOF){perror("Client-sscanf");}

  // check if there's a place available or the server is closed
  if (place >= 0)
    printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, place, IAMIN);
  else{
    if(pthread_mutex_lock(&mut)!=0){perror("Client-MutexLock");}
    serverOpen.x = 0;
    if(pthread_mutex_unlock(&mut)!=0){perror("Client-MutexUnLock");}
    printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, place, CLOSD);
  }

  // cleanup
  if(close(fd_priv)==-1){perror("Client-closePrivateFifo");}
  if (unlink(privateFifo)==-1){perror("Error destroying private fifo:");}
  pthread_exit(0);
}

int main(int argc, char* argv[], char *envp[]) {
  char fifoname[BUFSIZE];   /**< public fifo file name */
  char fifopath[BUFSIZE]="tmp/";  /**< public fifo path */
  double nsecs;  /**< numbers of seconds the program will be running */
  pthread_t tid;  /**< array to store thread id's */
  serverOpen.x = 1;

  //check arguments
  if (argc!=4) {
    printf("Usage: U1 <-t secs> fifoname\n");
    exit(1);}
  
  //read arguments
  strcpy(fifoname,argv[3]);
  nsecs=atoi(argv[2])*1000;
  strcat(fifopath,fifoname);

  //start counting time
  startTime();
  srand(time(NULL));

  //ciclo de geracao de pedidos
  while(elapsedTime() < (double) nsecs){
    if(pthread_mutex_lock(&mut)!=0){perror("Client-MutexLock");}
    if (serverOpen.x == 0) {
      if(pthread_mutex_unlock(&mut)!=0){perror("Client-MutexUnLock");}
      break;
    }
    if(pthread_mutex_unlock(&mut)!=0){perror("Client-MutexUnLock");}
    if(pthread_create(&tid,NULL, thread_func, (void *)fifopath)!=0){perror("Client-pthread_Create");}
    if(pthread_detach(tid)!= 0){perror("Client-pthread_join");}
  
    if(usleep(INTMS*1000) == -1){perror("Client-usleep");}
  }
  pthread_exit(0);
}
