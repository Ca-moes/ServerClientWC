#!/bin/bash

cd src2
# $? = 0 se compilou bem
# $? = 2 otherwise
make -s
if [ $? -eq 0 ] ; then
  ./tests.bash -q 1 -u 1 -l 1 -n 1 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 2 -u 3 -l 5 -n 3 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 5 -u 5 -l 5 -n 5 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 2 -u 5 -l 3 -n 4 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 10 -u 10 -l 7 -n 9 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 10 -u 5 -l 10 -n 10 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 15 -u 10 -l 12 -n 11 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 30 -u 35 -l 20 -n 15 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 70 -u 80 -l 25 -n 30 -f fifo.server | tee -a result.log
    echo "----------------------------"
  make clean
else
  echo "MAKE ERROR";
fi