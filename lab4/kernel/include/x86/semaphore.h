#ifndef __SEMA_H__
#define __SEMA_H__

#include "proc.h"

typedef int sem_t;

struct SemNode {
    PCB *proc;
};

#define MAX_PROCQUEUE_SIZE 20

struct ProcQueue {
    struct SemNode procQueue[MAX_PROCQUEUE_SIZE];
    int head, tail;
    int size;
};

struct Semaphore {
    int value;
    struct ProcQueue queue;
    sem_t next;
};

typedef struct Semaphore Semaphore;

void initSem();
int semCreate(sem_t *usr_sem, int value);
int semPost(sem_t *semID);
int semWait(sem_t *semID, PCB *waitPcb);
int semDestroy(sem_t *sem);
#endif