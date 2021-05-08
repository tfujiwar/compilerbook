// A warkaround to use standard libraries without including their headers
#define bool int
#define true 1
#define false 0
#define size_t int
#define NULL 0
#define SEEK_SET 0
#define SEEK_END 2

typedef struct {} FILE;

int printf();
int sprintf();
void *calloc();
int strcmp();
size_t strlen();
int memcmp();
bool isspace();
FILE *fopen();
int fclose();
int fseek();
size_t ftell();
size_t fread();
char* strerror();

extern int errno;
