#!/bin/bash

cd src
# $? = 0 se compilou bem
# $? = 2 otherwise
make -s
if [ $? -eq 0 ] ; then
  ./tests.bash -q 1 -u 1 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 2 -u 3 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 5 -u 5 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 2 -u 5 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 10 -u 10 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 10 -u 5 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 15 -u 10 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 30 -u 35 -f fifo.server | tee -a result.log
  echo "----------------------------"
  ./tests.bash -q 70 -u 80 -f fifo.server | tee -a result.log
    echo "----------------------------"
  make clean
else
  echo "MAKE ERROR";
fi