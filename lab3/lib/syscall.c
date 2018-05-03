#include "lib.h"
#include<stdarg.h>
#define	SYS_write	1 
#define SYS_fork	2
#define SYS_sleep	3
#define SYS_exit	4

int32_t syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx){
	int32_t ret = -1;
	asm volatile("int $0x80": "=a"(ret) : "a"(eax), "b"(ebx), "c"(ecx), "d"(edx));
	return ret;
}

int write(int fd, const void *buf, size_t count) {
    return syscall(SYS_write, (uint32_t)fd, (uint32_t)buf, (uint32_t)count);
}

unsigned int sleep(unsigned int seconds) {
	// syscall(SYS_write,seconds,0,0);
    return syscall(SYS_sleep, seconds, 0, 0);
}

pid_t fork(){
	return syscall(SYS_fork, 0, 0, 0);
}

void exit(int status){
	syscall(SYS_exit, status, 0, 0);
}
