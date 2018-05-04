#include "x86.h"
#include "device.h"
#include "klib.h"
#include "debug.h"
#include<stdarg.h>

struct TrapFrame *sys_write(struct TrapFrame *tf){	
    // only enable stdout
    int fd = (int)SYSCALL_ARG2(tf);	
    assert(fd == 1); 
    // Attention: base is necessary!
    const char *buf = (const char *)SYSCALL_ARG3(tf) + (uint32_t)pcb[current].base;
    size_t len = (size_t)SYSCALL_ARG4(tf); 

    int i;
    for (i = 0; i < len; ++i) {
        putChar(buf[i]);
        video_print(buf[i]);
    }
        
    SYSCALL_ARG1(tf) = i;
    return tf;
}

struct TrapFrame *GProtectFaultHandle(struct TrapFrame *tf){
    printf("GProtectFaultHandle\n");
    assert(0);
    return tf;
}

struct TrapFrame *sys_fork(struct TrapFrame *tf){
    // printf("pretend fork \n");
    assert(current != -1);
    // pcb[current].tf = tf;
    printf("in sys_fork\n");
    forkProc(&pcb[current]);
    return switchProc(schedule());
}

struct TrapFrame *sys_sleep(struct TrapFrame *tf){
    unsigned int seconds = SYSCALL_ARG2(tf);
    //printf("pretend sleep %d secondes\n",seconds);
    assert(current != -1);
    // pcb[current].tf = tf;
    sleepProc(&pcb[current], seconds);
    return switchProc(schedule());
}

struct TrapFrame *sys_exit(struct TrapFrame *tf){
    unsigned int status = SYSCALL_ARG2(tf);
    printf("exit, status = %d\n",status);
    killProc(current);
    return switchProc(schedule());
    
}

struct TrapFrame *syscallHandle(struct TrapFrame *tf) {
    /* 实现系统调用*/
    // #define	SYS_write	1 
    // #define SYS_fork 	2
    // #define SYS_sleep	3
    // #define SYS_exit	    4
    switch(SYSCALL_ARG1(tf)) {
        case SYS_write: return sys_write(tf); 	break;
        case SYS_fork:	return sys_fork(tf);	break;
        case SYS_sleep: return sys_sleep(tf);	break;
        case SYS_exit:  return sys_exit(tf);	break;
        /* we can add more syscall */
        default: assert(0);
    }
    return tf;
}

struct TrapFrame *timerInterruptHandle(struct TrapFrame *tf){
    Log("timerInterruptHandle");
    timeReduceProc();
    if (pcb[current].timeCount == 0)
        return switchProc(schedule());
    return tf;
}

struct TrapFrame *irqHandle(struct TrapFrame *tf) {
    /*
     * 中断处理程序
     */
    /* Reassign segment register */

    // in include/x86/irq.h
    // #define IRQ_EMPTY           -1
    // #define IRQ_G_PROTECT_FAULT 0xd
    // #define IRQ_TIMER           0x20
    // #define IRQ_SYSCALL         0x80
    //printf("%s\n","123");
    //printf("%d,%d,%s,%d\n",tf->eax,tf->ebx,tf->ecx,tf->edx);
    Log("irq:%x",tf->irq);

    // Reassign segment register 
    asm volatile("movl %0, %%eax" ::"r"(KSEL(SEG_KDATA)));
    asm volatile("movw %ax, %ds");
    asm volatile("movw %ax, %es");
    asm volatile("movw %ax, %fs");
    asm volatile("movl %0, %%eax" ::"r"(KSEL(SEG_VIDEO)));
    asm volatile("movw %ax, %gs");

    switch(tf->irq) {
        case IRQ_EMPTY: 
            return tf;
        case IRQ_G_PROTECT_FAULT:
            return GProtectFaultHandle(tf);
        case IRQ_TIMER:
            return timerInterruptHandle(tf);
        // 	return …… //todo
        
        case IRQ_SYSCALL:
            return syscallHandle(tf);
        default:assert(0);
    }
    return tf;
}



