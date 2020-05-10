#!/bin/bash
echo "╔══════════════════════════╗"
echo "║   CHANGE FLAGS IN BASH   ║"
echo "╚══════════════════════════╝"

cd src2/tmp
rm *.* &
cd ..

# $? = 0 se compilou bem
# $? = 2 otherwise
make -s
if [ $? -eq 0 ] ; then
  ./Q2 -t 5 -n 5 fifoname | tee Q2run.log &    # Un <-t nsecs> fifoname
  P1=$!
  ./U2 -t 5 fifoname | tee U2run.log &     # Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
  P2=$!
  wait $P1 $P2
  echo "END OF SERVER/CLIENT"
  make clean
else
  echo "MAKE ERROR";
fi

nIWANT=`grep IWANT U2run.log | wc -l`
nRECVD=`grep RECVD Q2run.log | wc -l`

nENTER=`grep ENTER Q2run.log | wc -l`
nIAMIN=`grep IAMIN U2run.log | wc -l`

nFAILD=`grep FAILD U2run.log | wc -l`
nGAVUP=`grep GAVUP Q2run.log | wc -l`
n2LATE=`grep 2LATE Q2run.log | wc -l`
nCLOSD=`grep CLOSD U2run.log | wc -l`

echo "IWANT : $nIWANT"
echo "RECVD : $nRECVD"
echo "ENTER : $nENTER"
echo "IAMIN : $nIAMIN"

# make clean
# Flags Possiveis:
# -t nsecs      - nº (aproximado) de segundos que o programa deve funcionar
# fifoname      - nome do canal público (FIFO) a criar pelo servidor para atendimento de pedidos
# -l nplaces    - lotação do Quarto de Banho
# -n nthreads   - nº (máximo) de threads a atender pedidos 