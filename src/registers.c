#include "registers.h"

void printRegister(double inst, int i, pid_t pid, pid_t tid, double long dur, size_t pl, char *oper) {
    printf("%f ; %u ; %u ; %u ; %Lf ; %lu ; %s\n", inst, i, pid, tid, dur, pl, oper);
    fflush(stdout);
}