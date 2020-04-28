#!/bin/bash
echo "╔══════════════════════════╗"
echo "║   CHANGE FLAGS IN BASH   ║"
echo "╚══════════════════════════╝"
cd src
# $? = 0 se compilou bem
# $? = 2 otherwise

make -f makefile.U1
if [ $? -eq 0 ] ; then
  ./U1 -t 2 fifoname  $@     # Un <-t nsecs> fifoname
  rm *.o
  rm U1
else
  echo "U1 MAKE ERROR";
fi


# Flags Possiveis:
# -t nsecs      - nº (aproximado) de segundos que o programa deve funcionar
# fifoname      - nome do canal público (FIFO) a criar pelo servidor para atendimento de pedidos
# -l nplaces    - lotação do Quarto de Banho
# -n nthreads   - nº (máximo) de threads a atender pedidos 
