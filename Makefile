CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mycc: $(OBJS)
	$(CC) -o mycc $(OBJS) $(LDFLAGS)

$(OBJS): mycc.h

test: mycc
	cc test/main.c -o tmp && ./tmp;
	./mycc -f test/main.c > tmp.s && cc tmp.s -o tmp && ./tmp;

test_queue: mycc
	cc test/queue.c -o tmp && ./tmp;
	./mycc -f test/queue.c > tmp.s && cc tmp.s -o tmp && ./tmp;

clean:
	rm -f mycc *.o *~ tmp*

.PHONY: test test_queue clean
