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

typedef struct bit {unsigned x:1;} bit; /**< bit Data Type */

int places[4];  /**< array of bits to store used places */
//com o intervalo entre pedidos 10 ms e valor maximo de uso 1000 ms, o valor maximo de lugares em uso
//ao mesmo tempo é 100. 100/32 =3.125 faz com que seja preciso tamanho 4
bit closed; /**< bit to store if Server is open or closed */

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; /**< mutex to access places[] */

    double nsecs;  /**< numbers of seconds the program will be running */


void * thread_func(void *arg){
    char * request = (char *) arg; /**< Request string received from public fifo*/
    int threadi, pid, dur, place;  /**< Component of request*/
    long tid;  /**< thread id of client */

    // parse contents received from public fifo
    sscanf(request,"[ %d, %d, %lu, %d, %d ]",&threadi, &pid, &tid, &dur, &place);
    printRegister(elapsedTime(), threadi, getpid(), pthread_self(), dur, -1, RECVD);

    // make private fifo pathname
    char privateFifo[BUFSIZE]="tmp/";
    char temp[BUFSIZE];
    sprintf(temp,"%d",pid);
    strcat(privateFifo,temp);
    strcat(privateFifo,".");
    sprintf(temp,"%ld",tid);
    strcat(privateFifo,temp);

    // open private fifo
    int fd_priv;
    do{fd_priv = open(privateFifo, O_WRONLY);}while(fd_priv==-1);
    if (fd_priv < 0) {
        printRegister(elapsedTime(), threadi, pid, pthread_self(), dur, place, GAVUP);
        pthread_exit((void *)1);
    }
    
    // Finding available place
    int tmp=0;
    pthread_mutex_lock(&mut); 
    while(TestBit(places,tmp)){tmp++;}
    place=tmp;
    SetBit(places, place);
    pthread_mutex_unlock(&mut); 

    // sending message with place to private fifo
    char sendMessage[BUFSIZE];

    if (elapsedTime() <= nsecs) {
        // always some place available
        sprintf(sendMessage,"[ %d, %d, %ld, %d, %d ]", threadi, getpid(), pthread_self(), dur, place);
        printRegister(elapsedTime(), threadi, getpid(), pthread_self(), dur, place, ENTER);
    }

    else {
        closed.x=1;
        sprintf(sendMessage,"[ %d, %d, %ld, %d, %d ]", threadi, getpid(), pthread_self(), -1, -1);
        printRegister(elapsedTime(), threadi, getpid(), pthread_self(), -1, -1, TLATE);
        printf("elapsedTime(): %f\n", elapsedTime());
        printf("dur: %d\n", dur);
        printf("sum: %f\n", elapsedTime() + dur);
        printf("nsecs: %f\n", nsecs);
    }

    // checking if server is closed
    if(closed.x){place=-1;}
    write(fd_priv,&sendMessage,BUFSIZE);

    

    // wait using time
    usleep(dur*1000); 
    if (!closed.x) printRegister(elapsedTime(), threadi, getpid(), pthread_self(), dur, place, TIMUP);
    
    // cleanup
    ClearBit(places, place);
    close(fd_priv);
    unlink(privateFifo);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    char fifoname[BUFSIZE];  /**< public fifo file name */
    char fifopath[BUFSIZE]="tmp/";  /**< public fifo path */
    char clientRequest[BUFSIZE];  /**< string read from public fifo */
    pthread_t threads[THREADS_MAX];  /**< array to store thread id's */
    int thr=0;  /**< index for thread id / nº od threads created */
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
    if(mkfifo(fifopath,0660)<0){perror("Error creating public FIFO"); exit(1);}
    
    //start counting time
    startTime();
    
    // open public fifo
    int fd_pub = open(fifopath,O_RDONLY); // sem O_NONBLOCK aqui fica bloqueado à espera que cliente abra
    if (fd_pub==-1){perror("Error opening public FIFO: "); exit(1);}
    
    // while loop to check running time
    while(elapsedTime() < (double) nsecs){        
        // while loop to check public fifo
        while(read(fd_pub,&clientRequest,BUFSIZE)<=0){ 
            if (elapsedTime() > (double) nsecs){
                close(fd_pub);
                unlink(fifopath);
                pthread_exit((void*)0);
            }
        }
        // create thread with contents of public fifo
        pthread_create(&threads[thr], NULL, thread_func, &clientRequest);
        pthread_detach(threads[thr]);
        thr++;
    }

    // cleanup
    closed.x=1;
    close(fd_pub);
    unlink(fifopath);
    pthread_exit((void*)0);
}
