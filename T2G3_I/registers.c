#include "registers.h"

void printRegister(long int inst, int i, pid_t pid, long tid, int dur, int pl, char *oper) {
  if(printf("%ld ; %u ; %d ; %lu ; %d ; %d ; %s\n", inst, i, pid, tid, dur, pl, oper)<0){perror("printRegister");}
  if(fflush(stdout) != 0){perror("fflush");}
}