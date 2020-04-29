#include "registers.h"

void printRegister(double inst, int i, pid_t pid, long tid, int dur, int pl, char *oper) {
    printf("%f ; %u ; %d ; %lu ; %d ; %d ; %s\n", inst, i, pid, tid, dur, pl, oper);
    fflush(stdout);
}