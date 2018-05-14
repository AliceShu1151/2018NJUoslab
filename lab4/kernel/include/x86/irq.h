#ifndef __IRQ_H__
#define __IRQ_H__

/* 中断处理相关函数 */
void initIdt(void);
void initIntr(void);

#define SYSCALL_ARG1(tf) ((tf)->eax)
#define SYSCALL_ARG2(tf) ((tf)->ebx)
#define SYSCALL_ARG3(tf) ((tf)->ecx)
#define SYSCALL_ARG4(tf) ((tf)->edx)

enum {
    SYS_write, SYS_fork, SYS_sleep, SYS_exit, 
    SYS_sem_init, SYS_sem_post, SYS_sem_wait, SYS_sem_destroy
};

#define IRQ_EMPTY           -1
#define IRQ_G_PROTECT_FAULT 0xd
#define IRQ_TIMER           0x20
#define IRQ_SYSCALL         0x80

#endif
