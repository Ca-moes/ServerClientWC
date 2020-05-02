CC = gcc
CFLAGS = -Wall -pthread
DEPS = utils.h registers.h Q1.h
OBJ = utils.o registers.o
TARGETS = U1 Q1

all: U1 Q1

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

U1: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm

Q1: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm

clean:
	@rm *.o $(TARGETS)