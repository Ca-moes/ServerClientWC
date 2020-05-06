CC = gcc
CFLAGS = -Wall -pthread
DEPS = utils.h registers.h Q1.h
OBJ = utils.o registers.o
TARGETS = U2 Q2

all: U2 Q2

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

U2: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm

Q2: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm

clean:
	@rm *.o $(TARGETS)