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
    assert(current != NILL);
    pcb[current].tf = tf;
    pcb[current].state = RUNNABLE;
    forkProc(&pcb[current]);
    return switchProc(schedule());
}

struct TrapFrame *sys_sleep(struct TrapFrame *tf){
    unsigned int seconds = SYSCALL_ARG2(tf);
    assert(current != NILL);
    pcb[current].tf = tf;
    Log("sleep: pcb%d\n",current);
    sleepProc(&pcb[current], seconds);
    printf("pcb %d state: %d\n",current,pcb[current].state);
    return switchProc(schedule());
}

struct TrapFrame *sys_exit(struct TrapFrame *tf){
    unsigned int status = SYSCALL_ARG2(tf);
    printf("exit, status = %d\n",status);
    pcb[current].tf = tf;
    killProc(current);
    return switchProc(schedule());
}

struct TrapFrame *syscallHandle(struct TrapFrame *tf) {
    // systerm call
    switch(SYSCALL_ARG1(tf)) {
        case SYS_write: return sys_write(tf); 	break;
        case SYS_fork:	return sys_fork(tf);	break;
        case SYS_sleep: return sys_sleep(tf);	break;
        case SYS_exit:  return sys_exit(tf);	break;
        // we can add more syscall 
        default: assert(0);
    }
    return tf;
}

extern int L;
extern int next[MAX_NPROC];

struct TrapFrame *timerInterruptHandle(struct TrapFrame *tf){
    Log("timerInterruptHandle");
    if (current == NILL) {
        current = 0;
        return pcb[current].tf;
    }
   
    if (pcb[current].state == RUNNING)
        pcb[current].state = RUNNABLE;
    
    pcb[current].timeCount--;
    for (int i = L; i != NILL; i = next[i]) {
        printf("before: pcb[%d] sleepTime: %d\n",i,pcb[i].sleepTime); 
        printf("before: pcb[%d] state: %d\n",i,pcb[i].state); 
        if (pcb[i].state == BLOCKED) {
            pcb[i].sleepTime--;
            if (pcb[i].sleepTime == 0)
                pcb[i].state = RUNNABLE;
        }
        
        printf("after: pcb[%d] sleepTime: %d\n",i,pcb[i].sleepTime);  
    }

    pcb[current].tf = tf;
    Log("current: %d",current);
    return switchProc(schedule());
}

struct TrapFrame *irqHandle(struct TrapFrame *tf) {

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
        // 	we can add more handles
        
        case IRQ_SYSCALL:
            return syscallHandle(tf);
        default:assert(0);
    }
    return tf;
}



