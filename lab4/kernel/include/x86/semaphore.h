#ifndef __SEMA_H__
#define __SEMA_H__

struct PCB;

struct Semaphore {
    int value;
    struct PCB *listStart;
    struct PCB *listEnd;
};

typedef int sem_t;
typedef struct Semaphore Semaphore;

void P(Semaphore *s);
void V(Semaphore *s);
int semInit(struct TrapFrame *tf);
#endif