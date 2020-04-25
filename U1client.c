#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include "utils.h"

#define BUFSIZE     256
#define THREADS_MAX 100

void * thread_func(){
    /*
        TODO: 
        criar fifo para comunicacao com server (publico e privado)
        gerar aleatoriamente parametros (tempo de uso o wc)
        terminar apos ter sido atendido pelo server

    */

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
    nsecs=atoi(argv[2])*10e6;

    //start counting time
    startTime();

    //ciclo de geracao de pedidos
    while(elapsedTime() < (double) nsecs){
        pthread_create(&threads[thr], NULL, thread_func, fifoname);
        pthread_join(threads[thr],NULL);
        
        thr++;
        sleep(2);
    }

    return 0;
}
