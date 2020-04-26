#include "register.h"

void printRegister(registers *registers) {
    printf("%d ; %u ; %u ; %u ; %Lf ; %u", 
    registers->inst, registers->i, registers->pid,
    registers->tid, registers->dur, registers->pl, registers->oper);
}