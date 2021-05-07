CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
OBJS_SELF=$(SRCS:.c=_self.o)

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
	@echo Build 2st Gen
	cc analyze.c -c -o analyze_self.o;
	cc codegen.c -c -o codegen_self.o;
	./mycc main.c > tmp.s && cc tmp.s -c -o main_self.o;
	cc parse.c -c -o parse_self.o;
	cc preprocess.c -c -o preprocess_self.o;
	cc tokenize.c -c -o tokenize_self.o;
	cc util.c -c -o util_self.o;
	cc -o mycc_2nd $(OBJS_SELF) $(LDFLAGS)

	@echo Build 3nd Gen
	cc analyze.c -c -o analyze_self.o;
	cc codegen.c -c -o codegen_self.o;
	./mycc_2nd main.c > tmp.s && cc tmp.s -c -o main_self.o;
	cc parse.c -c -o parse_self.o;
	cc preprocess.c -c -o preprocess_self.o;
	cc tokenize.c -c -o tokenize_self.o;
	cc util.c -c -o util_self.o;
	cc -o mycc_3rd $(OBJS_SELF) $(LDFLAGS)

	@echo Compare 2nd and 3rd Gen
	cmp mycc_2nd mycc_3rd

	@echo Succeed!!

clean:
	rm -f mycc mycc_* *.o *~ tmp*

.PHONY: test test_queue test_std clean
