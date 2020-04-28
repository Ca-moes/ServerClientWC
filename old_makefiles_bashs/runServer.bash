#!/bin/bash
echo "╔══════════════════════════╗"
echo "║   CHANGE FLAGS IN BASH   ║"
echo "╚══════════════════════════╝"
cd src
# $? = 0 se compilou bem
# $? = 2 otherwise

make -s -f makefile.Q1
if [ $? -eq 0 ] ; then
  rm *.o
  ./Q1 -t 10 fifoname $@     # Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
else
  echo "U1 MAKE ERROR";
fi

rm Q1

# Flags Possiveis:
# -t nsecs      - nº (aproximado) de segundos que o programa deve funcionar
# fifoname      - nome do canal público (FIFO) a criar pelo servidor para atendimento de pedidos
# -l nplaces    - lotação do Quarto de Banho
# -n nthreads   - nº (máximo) de threads a atender pedidos 
