# Pormenores de Implementação
Igual à parte 1: ambos os programas U2 e Q2 são multithreaded e funcionam com o máximo de paralelismo possível, juntamente com a verificação para não ocorrerem situações de _deadlock_, "colisão" ou "esperas ativas" com auxilio de mutexes. Os pedidos de acesso ao Quarto de Banho têm uma duração aleatória com limites entre **UPPERB** e **LOWERB** em U2.c e o intervalo entre pedidos é de **INTMS** = 50 ms.

Para a segunda parte o programa Q2 lê os novos argumentos -n e -l corretamente. Caso não seja especificado o número máximo de threads, este será **INT_MAX** e caso não seja especificado o número máximo de _places_, este será 32. Para chegar a este último valor reparamos que o **UPPERB** = 1000ms e o **INTMS** = 50ms o que faz com que, no pior caso possivel (todos os pedidos são de 1000ms) vai chegar a uma altura em que o número de lugares ocupados não aumenta mais porque ao ritmo a que os lugares ficam desocupados, ficam imediatamente ocupados outra vez. Com isto apenas 20 lugares (1000/50) podem estar ocupados em simultâneo. Como estamos a usar um array de _bits_ a partir de um array de _int's_ para guardar os lugares (explicação em baixo) e a fórmula `int sizearr = (int)ceil(nplaces/32.0);` para determinar o tamanho do array, podemos usar como valor default 32 lugares, o que dará um array de tamanho 1 (ou seja, uma variável _int_).

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

### Diferenças entre U1 e U2

Devido à ambiguidade do enunciado em relação a quando devem ser lançadas as flags 2LATE e CLOSD, às mudanças e correções de erros que tivemos que fazer em U2 após o feedback da primeira entrega e em geral, às alterações necessárias ao programa U para funcionar corretamente com o novo programa Q2, tivemos de alterar o programa original U1, indo contra o objetivo inicial de ter o programa Q2 a funcionar com o programa U1.
> 1 Note-se que, em rigor, não são precisos dois programas U: não há razão para o cliente U1 ter de ser diferente de U2!


#### Diferenças
1.  Estavamos a usar uma variavel global para manter _track_ dos _thread numbers_ que eram mandados para o **std_out** mas isto encorria em erro quando mais do que uma thread client tinha de escrever no mesmo instante no stdout porque ficavam com o mesmo valor. Para tratar disto criou-se uma variavel local da _thread number_ para guardar o seu número de thread global. 

![](https://i.imgur.com/x7Txhgt.png)

2. Na abertura do Fifo Pública era feita uma espera por parte do cliente e caso não tivesse resposta a thread fechava; Em U2 é feito _open()_ com a flag **O_NONBLOCK** com diversas tentativas e caso a thread não consiga abrir o fifo, significa que o servidor se encontra fechado. A thread cliente avisa o ciclo de criação de novas threads e fecha.

![](https://i.imgur.com/5qKLXDg.png)

3. A última mudança significativa é relativa à leitura do fifo privado. Em U1 eram feitas tentativas de leitura até dar erro e aí era mandado para o **std_out** a flag **FAILD**. Em U2 é feito um número fixo de tentativas na qual a thread gasta no total **UPPERB** ms em tentativas de leitura, relativo ao pior caso possivel em que uma thread servidor tem de esperar **UPPERB** ms para ter um lugar livre. Caso não haja resposta significa que houve um problema (poucos lugares disponiveis, sem thread para atender) e neste caso a thread cliente dá um _cleanup_ e fecha.

![](https://i.imgur.com/jiqAB6t.png)

## Detalhes - Q
- Os argumentos da linha de comando estão a ser lidos e guardados corretamente
- É criado o fifo público e aberto do lado do servidor sem **O_NONBLOCK**
- É lançada uma nova thread por cada pedido recebido a partir do fifo público, caso o número de threads que passarão a estar ativas não exceda o número de threads simultâneas possiveis obtido através dos argumentos.
- Em cada thread:
  - É aberto o fifo privado a partir das informações recebidas no fifo público,
  - Verifica-se se o servidor está fechado para imprimir **2LATE**
  - Se não estiver fechado, é feita uma pesquisa para descobrir um lugar livre,
  - Manda uma mensagem pelo fifo privado com a informação relativa ao **pl**,
  - Executa um *usleep()* com o tempo de uso para no fim notificar com **TIMUP** ou, se o servidor estiver fechado, fecha a thread sem fazer *usleep()*,
  - Faz o *cleanup* final e fecha o fifo privado
- Quando o tempo de execução do programa excede o tempo fornecido a partir dos argumentos, é feito em _main()_ uma leitura do fifo público para notificar os pedidos pendentes que o servidor se encontra fechado.
- É feito um _cleanup_ antes de sair da thread principal.

### Como guardar os lugares ocupados em memória?

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

### Verificação de lugares disponiveis

Para a seleção do lugar que a thread servidor designará para um cliente é feita uma pesquise sequencial dos lugares até encontrar um lugar livre (bit a 0). Caso o ciclo percorra todos os lugares e não encontre um lugar livre (ou seja, todos os lugares estão ocupados) recomeça o ciclo a partir do indice = 0. Assim que um lugar fique livre este é imediatamente designado para a thread client que estava à espera. Após encontrar um lugar livre é dado _unlock()_ ao mutex que permite aceder ao array **places** e caso haja threads à espera para procurar um novo lugar, uma dessas dará lock ao mutex e entrará no ciclo de procura/espera. 
```c
// Finding available place
int tmp=0;
if(pthread_mutex_lock(&mut)!=0){perror("Server-MutexLock");}
while(tmp < nplaces){
  if (TestBit(places,tmp) == 0){
    place=tmp;
    SetBit(places, place);
    break;
  }
  tmp++;
  if (tmp == nplaces)
    tmp=0;      
}
if(pthread_mutex_unlock(&mut)!=0){perror("Server-MutexUnLock");}
printRegister(time(NULL), threadi, getpid(), pthread_self(), dur, place, ENTER);
```

## Erros
No estado atual do programa não nos acontecem erros. Para testar isto alteramos o bash de testes da primeira entrega:

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
  valid2=1;
else
  echo -e "${RED}[FAILED] ${NC}FAILD"
fi

cd ..
# comment this line if you wish to keep the log files (debugging purposes)
# rm -rf logs

if [[ $valid1 -eq 1 && $valid2 -eq 1 ]] ; then
  exit 0;
else
  exit 1;
fi
```

São testadas 3 igualdades:
1. O número de **ENTER** tem de ser igual ao número de **IAMIN** - Para certificar que as threads client estão a esperar pela sua resposta enquanto as threads servidor procuram um lugar e que as threads client estão a ter receber uma resposta das threads servidor.
2. O número de **ENTER** tem de ser igual ao número de **TIMUP** - Para certificar que, independentemente de a casa de banho estar fechada ou não, as threads servidor esperam o tempo de uso para depois mandar a flag **TIMUP**.
3. O número de **FAILD** tem de ser igual à diferença entre o número de **IWANT** e a soma do número de **ENTER** com **2LATE** - A thread cliente pode tomar 2 caminhos, que dependem se ela recebe resposta a partir do fifo privado ou não. De todos os pedidos **IWANT**, alguns resultarão em **IAMIN** e outros em **FAILD**. o número de **FAILD** será então o número de pedidos que não receberam a resposta **ENTER** nem **2LATE**.

Deixamos de testar a igualdade entre **CLOSD** e **2LATE** após o feedback da primeira entrega. Agora raramente teremos **2LATE**'s que acontecem caso haja pedidos pendentes no fifo público que não foram processados no primeiro ciclo while do servidor. Os **CLOSD** aparecem quando a thread do cliente não consegue abrir o fifo público, isto devido à linha `if(unlink(fifopath)==-1){perror("Error destroying public fifo:");}` em Q2.c que, após o ciclo de geração de threads para atender os pedidos dentro do tempo de execução de Q, dá imediatemente unlink(), impossibilitando o programa cliente de escrever para o fifo público. Como não vai haver escrita, não haverá novas threads servidor criadas para mandar **2LATE**.

Para testar as variações possiveis de uso foi usado o seguinte bash como bateria de testes:
```bash
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
```

No final da execução deste bash todos os testes passavam e não havia entradas de erros nos ficheiros q1.err e u1.err, originando este ficheiro result.log:


<pre>mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 1sec
Setting client timeout to 1sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 2
ENTER: 2
GAVUP: 0
TIMUP: 2
2LATE: 0
--CLIENT--
IWANT: 20
IAMIN: 2
CLOSD: 0
FAILD: 18
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 2
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 2
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 18

----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 2sec
Setting client timeout to 3sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 11
ENTER: 11
GAVUP: 0
TIMUP: 11
2LATE: 0
--CLIENT--
IWANT: 40
IAMIN: 11
CLOSD: 2
FAILD: 29
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 11
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 11
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 29
----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 5sec
Setting client timeout to 5sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 54
ENTER: 54
GAVUP: 0
TIMUP: 54
2LATE: 0
--CLIENT--
IWANT: 100
IAMIN: 54
CLOSD: 0
FAILD: 46
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 54
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 54
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 46
----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 2sec
Setting client timeout to 5sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 10
ENTER: 10
GAVUP: 0
TIMUP: 10
2LATE: 0
--CLIENT--
IWANT: 40
IAMIN: 10
CLOSD: 2
FAILD: 30
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 10
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 10
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 30
----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 10sec
Setting client timeout to 10sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 139
ENTER: 139
GAVUP: 0
TIMUP: 139
2LATE: 0
--CLIENT--
IWANT: 200
IAMIN: 139
CLOSD: 0
FAILD: 61
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 139
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 139
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 61
----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 10sec
Setting client timeout to 5sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 85
ENTER: 85
GAVUP: 0
TIMUP: 85
2LATE: 0
--CLIENT--
IWANT: 100
IAMIN: 85
CLOSD: 0
FAILD: 15
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 85
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 85
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 15
----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 15sec
Setting client timeout to 10sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 184
ENTER: 184
GAVUP: 0
TIMUP: 184
2LATE: 0
--CLIENT--
IWANT: 200
IAMIN: 184
CLOSD: 0
FAILD: 16
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 184
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 184
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 16
----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 30sec
Setting client timeout to 35sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 598
ENTER: 598
GAVUP: 0
TIMUP: 598
2LATE: 0
--CLIENT--
IWANT: 599
IAMIN: 598
CLOSD: 2
FAILD: 1
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 598
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 598
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 1
----------------------------
mkdir: cannot create directory ‘logs’: File exists
Setting server timeout to 70sec
Setting client timeout to 80sec
SERVER/CLIENT RUNNING ...
END OF SERVER/CLIENT
--SERVER--
RECVD: 1397
ENTER: 1397
GAVUP: 0
TIMUP: 1397
2LATE: 0
--CLIENT--
IWANT: 1397
IAMIN: 1397
CLOSD: 2
FAILD: 0
<font color="#8AE234"><b>[PASSED] </b></font>ENTER - accepted requests: 1397
<font color="#8AE234"><b>[PASSED] </b></font>TIMEUP - number of timeup&apos;s: 1397
<font color="#8AE234"><b>[PASSED] </b></font>FAILD - number of faild&apos;s: 0
----------------------------
</pre>