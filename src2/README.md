# Pormenores de Implementação
Igual à parte 1: ambos os programas U2 e Q2 são multithreaded e funcionam com o máximo de paralelismo possível, juntamente com a verificação para não ocorrerem situações de _deadlock_, "colisão" ou "esperas ativas" com auxilio de mutexes. Os pedidos de acesso ao Quarto de Banho têm uma duração aleatória com limites entre **UPPERB** e **LOWERB** em U2.c e o intervalo entre pedidos é de **INTMS** = 50 ms.
Para a segunda parte o programa Q2 lê os novos argumentos

## Detalhes - U
- Os argumentos da linha de comando estão a ser lidos e guardados corretamente
- Após esta leitura os pedidos são gerados a partir de threads lançadas continuamente
- Em cada thread:
  - São gerados os valores de **i** e **dur**, 
  - É aberto o canal de comunicação para o servidor,
  - É escrita a mensagem no fifo público e com isto o fifo público é fechado,
  - É criado e aberto o fifo privado com o nome na forma _"pid.tid"_ em _/tmp_,
  - É lida a mensagem do fifo privado,
  - Verifica se o servidor está fechado (para parar de criar novos pedidos) ou se têm um lugar disponivel, caso o servidor esteja fechado é mandado para o **STDOUT** a flag **CLOSD** e uma variável global **serverOpen** muda de valor para notificar o ciclo de criação de pedidos em _main()_ para parar.
  - Fecha o fifo privado, elimina-o e faz o _clean up_ final antes de sair da thread 
- No fim do tempo de execução é feito um cleanup em _main()_

## Detalhes - Q
- Os argumentos da linha de comando estão a ser lidos e guardados corretamente
- É criado o fifo público e aberto do lado do servidor sem **O_NONBLOCK**
- É lançada uma nova thread por cada pedido recebido a partir do fifo público
- Em cada thread:
  - É aberto o fifo privado a partir das informações recebidas no fifo público,
  - É feita uma pesquisa para descobrir um lugar livre,
  - Manda uma mensagem pelo fifo privado com a informação relativa ao **pl**,
  - Executa um *usleep()* com o tempo de uso para no fim notificar com **TIMUP**,
  - Faz o *cleanup* final e fecha o fifo privado
- Quando o tempo de execução do programa excede o tempo fornecido a partir dos argumentos, é feito em _main()_ uma leitura do fifo público para notificar os pedidos pendentes que o servidor se encontra fechado.
- É feito um _cleanup_ antes de sair da thread principal.

Para guardar os lugares ocupados é apenas necessário saber a posição do lugar e se está ocupado ou não. Tendo como objetivo poupar o uso de memória, implementamos a partir de um array de inteiros, um array de bits, com cada bit a corresponder a um lugar. Caso esse bit esteja a 1, o lugar encontra-se ocupado. Isto foi realizado usando [este link](http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html) como fonte e com o auxilio das seguintes macros (Sendo **A** o array de *int*s e **k** a posição no array de bits):
```c
#define SetBit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define ClearBit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )
#define TestBit(A,k)    ( A[(k/32)] & (1 << (k%32)) )
```
Também com o objetivo de poupar espaço de memória está declarado em _registers.h_:
```c 
typedef struct bit {unsigned x:1;} bit; /**< bit Data Type */
```
esta struct que guarda valores booleanos (1 ou 0 neste caso) em um bit.

## Erros
No estado atual do programa não nos acontecem erros. Para testar isto usamos um bash de testes fornecido por um estudante do nosso ano (Gonçalo Teixeira - up201806562) que foi adaptado de um bash de testes criado por nós numa fase inicial do projeto:

```bash
#!/bin/bash
# Adapted from André Daniel Gomes's Test script
# usage|help|info: ./test -q <server timeout> -u <client timeout> -f <server fifo>

RED='\033[1;31m'
GREEN='\033[1;32m'
NC='\033[0m' # No Color

# default values
serverTime=10
clientTime=15
fifoname='fifo.server'

while getopts ":q:u:f:" opt; do
  case $opt in
    q) serverTime=$OPTARG;;
    u) clientTime=$OPTARG;;
    f) fifoname=$OPTARG;;
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

./Q1 -t $serverTime "$fifoname" > logs/q1.log 2> logs/q1.err &  # Un <-t nsecs> fifoname
P1=$!
./U1 -t $clientTime "$fifoname" > logs/u1.log 2> logs/u1.err &   # Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
P2=$!
wait $P1 $P2
echo "END OF SERVER/CLIENT"

cd logs || exit

nREQST=`grep IWANT u1.log | wc -l`
nFAILD=`grep FAILD u1.log | wc -l`
nGAVUP=`grep GAVUP q1.log | wc -l`

n2LATE=`grep 2LATE q1.log | wc -l`
nCLOSD=`grep CLOSD u1.log | wc -l`

nIAMIN=`grep IAMIN u1.log | wc -l`
nENTER=`grep ENTER q1.log | wc -l`

echo "Requests sent: $nREQST"
echo "Failed requests: $nFAILD"
echo "Gave up requests: $nGAVUP"

valid1=0;
valid2=0;

if  [ $n2LATE -eq $nCLOSD ] ; then
  echo -e "${GREEN}[PASSED] ${NC}2LATE - too late requests: $n2LATE"
  valid1=1;
else
  echo -e "${RED}[FAILED] ${NC}2LATE"
fi

if  [ $nIAMIN -eq $nENTER ] ; then
  echo -e "${GREEN}[PASSED] ${NC}ENTER - accepted requests: $nENTER"
  valid2=1;
else
  echo -e "${RED}[FAILED] ${NC}ENTER"
fi

cd ..
# comment this line if you wish to keep the log files (debugging purposes)
rm -rf logs

if [[ $valid1 -eq 1 && $valid2 -eq 1 ]] ; then
  exit 0;
else
  exit 1;
fi
```

Este bash foi usado juntamente com outro para testar com diversos valores de tempos de execução para ambos os programas:

```bash
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
```

No final da execução deste bash todos os testes passavam e não havia entradas de erros nos ficheiros q1.err e u1.err, originando este ficheiro result.log:


<pre>Setting server timeout to 1sec
Setting client timeout to 1sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 20
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 20

Setting server timeout to 2sec
Setting client timeout to 3sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 41
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 1
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 40
 
Setting server timeout to 5sec
Setting client timeout to 5sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 100
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 100

Setting server timeout to 2sec
Setting client timeout to 5sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 41
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 1
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 40

Setting server timeout to 10sec
Setting client timeout to 10sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 200
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 200

Setting server timeout to 10sec
Setting client timeout to 5sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 100
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 100

Setting server timeout to 15sec
Setting client timeout to 10sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 200
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 200

Setting server timeout to 30sec
Setting client timeout to 35sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 600
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 1
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 599

Setting server timeout to 70sec
Setting client timeout to 80sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
Requests sent: 1398
Failed requests: 0
Gave up requests: 0
<font color="#8AE234"><b>[PASSED] </b></font>2LATE - too late requests: 1
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 1397</pre>
