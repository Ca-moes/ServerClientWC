#!/bin/bash
# Adapted from André Daniel Gomes's Test script
# usage|help|info: ./test -q <server timeout> -u <client timeout> -f <server fifo>

RED='\033[1;31m'
GREEN='\033[1;32m'
NC='\033[0m' # No Color

# default values
serverTime=10
clientTime=15
nPlaces=50
nThreads=50
fifoname='fifo.server'

while getopts ":q:u:f:l:n:" opt; do
  case $opt in
    q) serverTime=$OPTARG;;
    u) clientTime=$OPTARG;;
    f) fifoname=$OPTARG;;
    l) nPlaces=$OPTARG;;
    n) nThreads=$OPTARG;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

mkdir logs
echo "Setting server timeout to "$serverTime"sec"
echo "Setting client timeout to "$clientTime"sec"
echo "SERVER/CLIENT RUNNING ..."

./Q2 -t $serverTime -l $nPlaces -n $nThreads "$fifoname" > logs/q2.log 2> logs/q2.err &  # Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
P1=$!
./U2 -t $clientTime "$fifoname" > logs/u2.log 2> logs/u2.err &   # Un <-t nsecs> fifoname
P2=$!
wait $P1 $P2
echo "END OF SERVER/CLIENT"

cd logs
nENTER=`grep ENTER q2.log | wc -l`
nIWANT=`grep IWANT u2.log | wc -l`
nFAILD=`grep FAILD u2.log | wc -l`
nGAVUP=`grep GAVUP q2.log | wc -l`
n2LATE=`grep 2LATE q2.log | wc -l`
nCLOSD=`grep CLOSD u2.log | wc -l`
nIAMIN=`grep IAMIN u2.log | wc -l`
nTIMUP=`grep TIMUP q2.log | wc -l`
nRECVD=`grep RECVD q2.log | wc -l`

echo "--SERVER--"
echo "RECVD: $nRECVD"
echo "ENTER: $nENTER"
echo "GAVUP: $nGAVUP"
echo "TIMUP: $nTIMUP"
echo "2LATE: $n2LATE"

echo "--CLIENT--"
echo "IWANT: $nIWANT"
echo "IAMIN: $nIAMIN"
echo "CLOSD: $nCLOSD"
echo "FAILD: $nFAILD"

valid1=0;
valid2=0;
valid3=0;

if  [ $nENTER -eq $nIAMIN ] ; then
  echo -e "${GREEN}[PASSED] ${NC}ENTER - accepted requests: $nENTER"
  valid1=1;
else
  echo -e "${RED}[FAILED] ${NC}ENTER"
fi

if  [ $nENTER -eq $nTIMUP ] ; then
  echo -e "${GREEN}[PASSED] ${NC}TIMEUP - number of timeup's: $nTIMUP"
  valid2=1;
else
  echo -e "${RED}[FAILED] ${NC}TIMEUP"
fi

if  [ $nFAILD -eq "$(echo "$nIWANT - ($nENTER + $n2LATE)" | bc)" ] ; then
  echo -e "${GREEN}[PASSED] ${NC}FAILD - number of faild's: $nFAILD"
  valid3=1;
else
  echo -e "${RED}[FAILED] ${NC}FAILD"
fi

cd ..
# comment this line if you wish to keep the log files (debugging purposes)
# rm -rf logs

if [[ $valid1 -eq 1 && $valid2 -eq 1 && $valid3 -eq 1 ]] ; then
  exit 0;
else
  exit 1;
fi