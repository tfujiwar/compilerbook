CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

test_file: 9cc
	./9cc -f test/main.c > tmp.s && cc tmp.s -o tmp && ./tmp;

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
