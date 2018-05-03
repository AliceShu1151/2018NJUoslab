#ifndef __lib_h__
#define __lib_h__
#include "types.h"

#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2
#define EOF             -1

// stdio.c
size_t strlen(const char *s);
void printf(const char *format,...);

// syscall.c
int write(int fd, const void *buf, size_t count);
pid_t fork();
unsigned int sleep(unsigned int seconds);
void exit(int status);
#endif
