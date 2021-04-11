#include <stdio.h>
#include <stdlib.h>

int foo() {
    printf("func 'foo' is called:\n");
    return 0;
}

int bar(int x, int y) {
    printf("func 'bar' is called: %d, %d\n", x, y);
    return x + y;
}

void alloc4(int **p, int a, int b, int c, int d) {
    *p = malloc(16);
    *(*p+0) = a;
    *(*p+1) = b;
    *(*p+2) = c;
    *(*p+3) = d;
}
