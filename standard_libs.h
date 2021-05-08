// A warkaround to use standard libraries without including their headers
#define bool int
#define true 1
#define false 0
#define size_t int
#define NULL 0

int printf();
int sprintf();
void *calloc();
int strcmp();
int strlen();
int isspace();