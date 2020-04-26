#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

typedef struct Operation {
    char **envip;
    char *dir;
    unsigned int IWANT : 1; // cliente faz o pedido inicial
    unsigned int RECVD : 1; // servidor acusa receção do pedido
    unsigned int ENTER : 1; // servidor diz que aceitou o pedido
    unsigned int IAMIN : 1; // cliente acusa a utilização do Quarto de banho
    unsigned int TIMUP : 1; // servidor diz que terminou o tempo de utilização
    unsigned int TLATE : 1; // servidor rejeita pedido por Quarto de banho já ter encerrado (2LATE)
    unsigned int CLOSD : 1; // cliente acusa informação de que o Quarto de banho está fechado
    unsigned int FAILD : 1; // cliente já não consegue receber proposta do servidor;
    unsigned int GAVUP : 1; // servidor já não consegue responder a pedido porque FIFO privado do cliente fechou
} operations;

void initOperations(operations *operations, char *envp[]);

#endif /*OPERATIONS_H*/