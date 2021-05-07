CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mycc: $(OBJS)
	$(CC) -o mycc $(OBJS) $(LDFLAGS)

$(OBJS): mycc.h standard_libs.h

test: mycc
	./mycc test/main.c > tmp.s && cc tmp.s -o tmp && ./tmp;
	cc test/main.c -o tmp -Wno-builtin-declaration-mismatch -Wno-cpp -Wno-switch-unreachable && ./tmp;

test_queue: mycc
	./mycc test/queue.c > tmp.s && cc tmp.s -o tmp && ./tmp;
	cc test/queue.c -o tmp -Wno-builtin-declaration-mismatch && ./tmp;

test_std: mycc
	./mycc test/std.c > tmp.s && cc tmp.s -o tmp && ./tmp;

test_self: mycc
	./mycc main.c > tmp.s && cc tmp.s -o tmp && ./tmp;

clean:
	rm -f mycc *.o *~ tmp*

.PHONY: test test_queue test_std clean
