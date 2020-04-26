#ifndef REGISTER_H
#define REGISTER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include "time.h"

#define IWANT "IWANT" // cliente faz o pedido inicial	
#define IAMIN "IAMIN" // cliente acusa a utilização do Quarto de banho	
#define CLOSD "CLOSD" // cliente acusa informação de que o Quarto de banho está fechado	
#define FAILD "FAILD" // cliente já não consegue receber proposta do servidor;	
#define RECVD "RECVD" // servidor acusa receção do pedido	
#define ENTER "ENTER" // servidor diz que aceitou o pedido	
#define TIMUP "TIMUP" // servidor diz que terminou o tempo de utilização	
#define TLATE "TLATE" // servidor rejeita pedido por Quarto de banho já ter encerrado (2LATE)	
#define GAVUP "GAVUP" // servidor já não consegue responder a pedido porque FIFO privado do cliente fechou

typedef struct Register {
    double inst; // valor retornado pela chamada ao sistema time(), na altura da produção da linha
    unsigned int i; // o número sequencial do pedido (gerado por Un)
    pid_t pid; // identificador de sistema do processo
                      // (cliente, no caso do pedido; servidor, no caso da resposta)
    int tid; // identificador no sistema do thread cliente
                      //(cliente, no caso do pedido; servidor, no caso da resposta)
    long double dur; // duração, em milissegundos, de utilização (de um lugar) do Quarto de Banho
                     // (valor atribuído no pedido e repetido na resposta, se se der a ocupação;
                     // se não se der, por motivo de o Quarto de Banho estar em vias de encerrar,
                     // o servidor responde aqui com o valor -1
    unsigned int pl; // no de lugar que eventualmente lhe será atribuído no Quarto de Banho
                     // (no pedido, este campo é preenchido com o valor -1 e na resposta terá 
                     // o valor do lugar efetivamente ocupado ou também -1, na sequência de
                     // insucesso de ocupação, por motivo de encerramento)
    char *oper; // siglas de 5 letras ajustadas às fases da operação que cada processo/ thread 
                // acabou de executar e que variam conforme se trate do cliente ou do servidor:
} registers;

void printRegister(double inst, int i, pid_t pid, int tid, double long dur, size_t pl, char *oper);

#endif /*REGISTER_H*/
