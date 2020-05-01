#!/bin/bash
echo "╔══════════════════════════╗"
echo "║       RUNNING TESTS      ║"
echo "╚══════════════════════════╝"
cd src/tmp
rm * &
cd ..
# $? = 0 se compilou bem
# $? = 2 otherwise
make -s
if [ $? -eq 0 ] ; then
  ./Q1 -t 10 door > q1.log 2> q1.err &  # Un <-t nsecs> fifoname
  P1=$!
  ./U1 -t 12 door > u1.log 2> u1.err &   # Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
  P2=$!
  wait $P1 $P2
  echo "END OF SERVER/CLIENT"

  n2LATE=`grep 2LATE q1.log | wc -l`
  echo $n2LATE
  nCLOSD=`grep CLOSD u1.log | wc -l`
  echo $nCLOSD

  make clean
else
  echo "MAKE ERROR";
fi

# make clean
# Flags Possiveis:
# -t nsecs      - nº (aproximado) de segundos que o programa deve funcionar
# fifoname      - nome do canal público (FIFO) a criar pelo servidor para atendimento de pedidos
# -l nplaces    - lotação do Quarto de Banho
# -n nthreads   - nº (máximo) de threads a atender pedidos 
