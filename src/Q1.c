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
    char * request = (char *) arg;

    int threadi, pid, dur, place;
    long int tid;
    sscanf(request,"[%d, %d, %ld, %d, %d]",&threadi, &pid, &tid, &dur, &place);
    printf("thread received: %s\n",request); //just testing

    /*
    TODO:
        - esperar que haja lugar disponivel e prosseguir (há sempre - 1 parte)
            - portanto, percorrer os lugares do wc e se nao houver lugar acrescenta
        - verificar se da tempo de o cliente usar a wc antes de fechar (se não enviar -1 em dur)
        - controlar o tempo de utilização (esta thread so termina qnd o cliente acabar de usar)
        - enviar informacao para o cliente com place atualizado (-1 caso nao de para usar, >1 caso dê)
    */

  return NULL;
}

int main(int argc, char* argv[]) {
    char fifoname[BUFSIZE];
    char fifopath[BUFSIZE]="tmp/";
    double nsecs;
    pthread_t threads[THREADS_MAX];
    int thr=0;

    printf("here0\n");
    if (argc!=4) {
        printf("Usage: U1 <-t secs> fifoname\n");
        exit(1);
    }
    printf("here1\n");

    //read arguments
    strcpy(fifoname,argv[3]);
    nsecs=atoi(argv[2])*1000;
    strcat(fifopath,fifoname);

    printf("here2\n");
    //create public fifo
    if(mkfifo(fifopath,0660)<0){perror("Error creating public FIFO"); exit(1);}
    printf("here3\n");
    sleep(5);
    //start counting time
    startTime();
    printf("here4\n");
    int fd_pub = open(fifopath,O_RDONLY); // sem O_NONBLOCK aqui fica bloqueado à espera que cliente abra
    printf("here5\n");
    if (fd_pub==-1){perror("Error opening public FIFO: "); exit(1);}
    printf("here6\n");
    char clientRequest[BUFSIZE];
    while(elapsedTime() < (double) nsecs){
        printf("1 elapsed: %f nsecs: %f\n",elapsedTime(),nsecs);
        
        while(read(fd_pub,&clientRequest,BUFSIZE)<=0){ //loop ate encontrar algo p ler
            sleep(1);

            if (elapsedTime() > (double) nsecs){
                close(fd_pub);
                unlink(fifopath);
                return 0;
            }
        }

        pthread_create(&threads[thr], NULL, thread_func, &clientRequest);
        pthread_join(threads[thr],NULL);
        thr++;
    }

    close(fd_pub);
    unlink(fifopath);
    return 0;
}