#include "lib.h"
#include<stdarg.h>

enum {
    SYS_write, SYS_fork, SYS_sleep, SYS_exit, 
    SYS_sem_init, SYS_sem_post, SYS_sem_wait, SYS_sem_destroy
};

int32_t syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
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

pid_t fork() {
	return syscall(SYS_fork, 0, 0, 0);
}

void exit(int status) {
	syscall(SYS_exit, status, 0, 0);
}


sem_t sem_init(sem_t *sem, uint32_t value) {
	return syscall(SYS_sem_init, (uint32_t)sem, value, 0);
}

sem_t sem_post(sem_t *sem) {
	return syscall(SYS_sem_post, (uint32_t)sem, 0, 0);
}

sem_t sem_wait(sem_t *sem) {
	return syscall(SYS_sem_wait, (uint32_t)sem, 0, 0);
}

sem_t sem_destroy(sem_t *sem) {
	return syscall(SYS_sem_destroy, (uint32_t)sem, 0, 0);
}