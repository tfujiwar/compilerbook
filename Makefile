CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mycc: $(OBJS)
	$(CC) -o mycc $(OBJS) $(LDFLAGS)

$(OBJS): mycc.h

test: mycc
	./test.sh

test_file: mycc
	./mycc -f test/main.c > tmp.s && cc tmp.s -o tmp && ./tmp;

clean:
	rm -f mycc *.o *~ tmp*

.PHONY: test clean
