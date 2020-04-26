#ifndef REGISTER_H
#define REGISTER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "time.h"
#include "operations.h"

typedef struct Register {
    double inst; // valor retornado pela chamada ao sistema time(), na altura da produção da linha
    unsigned int i; // o número sequencial do pedido (gerado por Un)
    unsigned int pid; // identificador de sistema do processo
                      // (cliente, no caso do pedido; servidor, no caso da resposta)
    unsigned int tid; // identificador no sistema do thread cliente
                      //(cliente, no caso do pedido; servidor, no caso da resposta)
    long double dur; // duração, em milissegundos, de utilização (de um lugar) do Quarto de Banho
                     // (valor atribuído no pedido e repetido na resposta, se se der a ocupação;
                     // se não se der, por motivo de o Quarto de Banho estar em vias de encerrar,
                     // o servidor responde aqui com o valor -1
    unsigned int pl; // no de lugar que eventualmente lhe será atribuído no Quarto de Banho
                     // (no pedido, este campo é preenchido com o valor -1 e na resposta terá 
                     // o valor do lugar efetivamente ocupado ou também -1, na sequência de
                     // insucesso de ocupação, por motivo de encerramento)
    operations oper; // siglas de 5 letras ajustadas às fases da operação que cada processo/ thread 
                // acabou de executar e que variam conforme se trate do cliente ou do servidor:
} registers;

void printRegister(registers *registers);

#endif /*REGISTER_H*/