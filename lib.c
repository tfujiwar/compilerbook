#include <stdio.h>
int foo() {
    printf("func 'foo' is called:\n");
    return 0;
}

int bar(int x, int y) {
    printf("func 'bar' is called: %d, %d\n", x, y);
    return x + y;
}
