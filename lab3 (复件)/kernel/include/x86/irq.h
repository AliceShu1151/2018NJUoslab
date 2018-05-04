#ifndef __IRQ_H__
#define __IRQ_H__

/* 中断处理相关函数 */
void initIdt(void);
void initIntr(void);

#define SYSCALL_ARG1(tf) ((tf)->eax)
#define SYSCALL_ARG2(tf) ((tf)->ebx)
#define SYSCALL_ARG3(tf) ((tf)->ecx)
#define SYSCALL_ARG4(tf) ((tf)->edx)

#define	SYS_write	1 
#define SYS_fork	2
#define SYS_sleep	3
#define SYS_exit	4

#define IRQ_EMPTY           -1
#define IRQ_G_PROTECT_FAULT 0xd
#define IRQ_TIMER           0x20
#define IRQ_SYSCALL         0x80

#endif
