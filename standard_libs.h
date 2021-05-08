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
int vfprintf();
int fprintf();
void *calloc();
void *malloc();
void *realloc();
int strcmp();
int strncmp();
size_t strlen();
char *strchr();
int strncpy();
int memcmp();
bool isspace();
FILE *fopen();
int fclose();
int fseek();
size_t ftell();
size_t fread();
char* strerror();
bool isdigit();
int strtol();
void exit();

typedef void *va_list;
void va_start();
void va_end();

#define FOPEN_MAX 1024
extern int errno;
extern FILE __streams[FOPEN_MAX];
#define stderr &__streams[2]