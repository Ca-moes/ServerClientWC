#include "registers.h"

void printRegister(long int inst, int i, pid_t pid, long tid, int dur, int pl, char *oper) {
    printf("%ld ; %u ; %d ; %lu ; %d ; %d ; %s\n", inst, i, pid, tid, dur, pl, oper);
    fflush(stdout);
}