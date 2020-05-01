#!/bin/bash

# Adapted from André Daniel Gomes's Test script

echo "╔══════════════════════════╗"
echo "║       RUNNING TESTS      ║"
echo "╚══════════════════════════╝"

# $? = 0 se compilou bem
# $? = 2 otherwise

echo "Compiling U1 and Q1 ..."
cd src
make

if [ $? -eq 0 ] ; then
  mkdir logs
  echo "Setting server timeout to 20sec"
  echo "Setting client timeout to 15sec"
  echo "SERVER/CLIENT RUNNING ..."

  ./Q1 -t 15 server.fifo > logs/q1.log 2> logs/q1.err &  # Un <-t nsecs> fifoname
  P1=$!
  ./U1 -t 20 server.fifo > logs/u1.log 2> logs/u1.err &   # Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
  P2=$!
  wait $P1 $P2
  echo "END OF SERVER/CLIENT"

  cd logs || exit

  nREQST=`grep IWANT u1.log | wc -l`
  nFAILD=`grep FAILD u1.log | wc -l`

  n2LATE=`grep 2LATE q1.log | wc -l`
  nCLOSD=`grep CLOSD u1.log | wc -l`

  nIAMIN=`grep IAMIN u1.log | wc -l`
  nENTER=`grep ENTER q1.log | wc -l`

  echo "Requests sent: $nREQST"

  if  [ $n2LATE -eq $nCLOSD ] ; then
    echo "[PASSED] 2LATE - too late requests: $n2LATE"
  else
    echo "[FAILED] 2LATE"
  fi

  if  [ $nIAMIN -eq $nENTER ] ; then
    echo "[PASSED] ENTER - accepted requests: $nENTER"
  else
    echo "[FAILED] ENTER"
  fi

  echo "Failed requests: $nFAILD"

  cd ..
  rm -rf logs
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
