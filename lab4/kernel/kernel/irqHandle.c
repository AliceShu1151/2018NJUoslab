#include "x86.h"
#include "device.h"
#include "klib.h"
#include "debug.h"
#include<stdarg.h>

struct TrapFrame *sys_write(struct TrapFrame *tf) {	
    // only enable stdout
    int fd = (int)SYSCALL_ARG2(tf);	
    assert(fd == 1); 
    // Attention: base is necessary!
    const char *buf = (const char *)SYSCALL_ARG3(tf) + (uint32_t)current->base;
    size_t len = (size_t)SYSCALL_ARG4(tf); 

    int i;
    for (i = 0; i < len; ++i) {
        putChar(buf[i]);
        video_print(buf[i]);
    }
        
    SYSCALL_ARG1(tf) = i;
    return tf;
}

struct TrapFrame *GProtectFaultHandle(struct TrapFrame *tf) {
    printf("GProtectFaultHandle\n");
    assert(0);
    return tf;
}

struct TrapFrame *sys_fork(struct TrapFrame *tf) {
    assert(current != NULL);
    current->tf = tf;
    current->state = RUNNABLE;
    forkProc(current);
    return switchProc(schedule());
}

struct TrapFrame *sys_sleep(struct TrapFrame *tf) {
    unsigned int seconds = SYSCALL_ARG2(tf);
    assert(current != NULL);
    current->tf = tf;
    sleepProc(current, seconds);
    return switchProc(schedule());
}

struct TrapFrame *sys_exit(struct TrapFrame *tf) {
    unsigned int status = SYSCALL_ARG2(tf);
    printf("exit, status = %d\n",status);
    current->tf = tf;
    current->state = DEAD;
    PCB *next = schedule();
    destroyProc(current);
    return switchProc(next);
}

struct TrapFrame *sys_sem_init(struct TrapFrame *tf) {
    int value = SYSCALL_ARG3(tf);
    sem_t *usr_sem = (sem_t *)SYSCALL_ARG2(tf) + (uint32_t)current->base;
    SYSCALL_ARG1(tf) = semCreate(usr_sem, value);
    return tf;
}

struct TrapFrame *sys_sem_post(struct TrapFrame *tf) {
    sem_t *semID = (sem_t *)SYSCALL_ARG2(tf) + (uint32_t)current->base;
    SYSCALL_ARG1(tf) = semPost(semID);
    Log("user_semID: %d", *semID);
    return tf;
}

struct TrapFrame *sys_sem_wait(struct TrapFrame *tf) {
    sem_t *semID = (sem_t *)SYSCALL_ARG2(tf) + (uint32_t)current->base;
    SYSCALL_ARG1(tf) = semWait(semID, current);
    if (current->state == BLOCKED) 
        return  switchProc(schedule());
    return tf;
}

struct TrapFrame *sys_sem_destroy(struct TrapFrame *tf) {
    sem_t *freeSem = (sem_t *)SYSCALL_ARG2(tf) + (uint32_t)current->base;
    semDestroy(freeSem);
    return tf;
}

struct TrapFrame *syscallHandle(struct TrapFrame *tf) {
    // systerm call
    if(SYSCALL_ARG1(tf) != SYS_write)
        Log("tf->eax: %d", SYSCALL_ARG1(tf));
    switch(SYSCALL_ARG1(tf)) {
        case SYS_write:         return sys_write(tf); 	    break;
        case SYS_fork:	        return sys_fork(tf);	    break;
        case SYS_sleep:         return sys_sleep(tf);	    break;
        case SYS_exit:          return sys_exit(tf);	    break;
        case SYS_sem_init:      return sys_sem_init(tf);    break;
        case SYS_sem_post:      return sys_sem_post(tf);    break;
        case SYS_sem_wait:      return sys_sem_wait(tf);    break;
        case SYS_sem_destroy:   return sys_sem_destroy(tf); break;
        // we can add more syscall 
        default: assert(0);
    }
    return tf;
}

struct TrapFrame *timerInterruptHandle(struct TrapFrame *tf) {
    if (current == NULL) {
        current = idle;
        return current->tf;
    }
   
    if (current->state == RUNNING)
        current->state = RUNNABLE;
    
    timeReduceProc();

    current->tf = tf;
    return switchProc(schedule());
}

struct TrapFrame *irqHandle(struct TrapFrame *tf) {
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



