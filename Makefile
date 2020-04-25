CC = gcc
CFLAGS = -Wall -pthread
OBJS = U1client.o utils.o
XHDRS = utils.h
EXEC = U1

$(EXEC): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $@ -lm

%.o: %.c %.h $(XHDRS)
	@$(CC) $(CFLAGS) -c $<

.PHONY : clean
clean :
	@-rm $(OBJS) $(EXEC)
