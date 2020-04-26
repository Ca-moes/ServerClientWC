#!/bin/bash
echo "╔══════════════════════════╗"
echo "║   CHANGE FLAGS IN BASH   ║"
echo "╚══════════════════════════╝"
cd src
# $? = 0 se compilou bem
# $? = 2 otherwise

make -s -f makefile.Q1
if [ $? -eq 0 ] ; then
  make -s -f makefile.U1
  if [ $? -eq 0 ] ; then
    ./Q1 -t 10 fifoname $@   # Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
    ./U1 -t 2 fifoname       # Un <-t nsecs> fifoname
  else
    echo "U1 MAKE ERROR";
  fi
else
  echo "Q1 MAKE ERROR";
fi

rm *.o
rm Q1
rm U1

# Flags Possiveis:
# -t nsecs      - nº (aproximado) de segundos que o programa deve funcionar
# fifoname      - nome do canal público (FIFO) a criar pelo servidor para atendimento de pedidos
# -l nplaces    - lotação do Quarto de Banho
# -n nthreads   - nº (máximo) de threads a atender pedidos 
