#include "x86.h"
#include "klib.h"
#include "device.h"
#include "debug.h"

#define MAX_NSEMA 10

struct Semaphore sem[MAX_NSEMA];
sem_t semID;

sem_t semAlloc();
void semFree(sem_t freeSem);
void procQueueInit(struct ProcQueue *queue);
PCB *procQueuePop(struct ProcQueue *queue);
PCB *procQueuePush(struct ProcQueue *queue, PCB *pushPcb);
void P(Semaphore *s, PCB *waitPcb);
void V(Semaphore *s);

void initSem() {
    semID = 0;
    for (int i = 0; i < MAX_NSEMA - 1; i++) 
        sem[i].next = i + 1;
    sem[MAX_NSEMA - 1].next = -1;
}

sem_t semAlloc() {
    if (semID == -1) {
        printf("No semaphore for alloc !");
        assert(0);
    }
    sem_t ret = semID;
    semID = sem[semID].next;  
    return ret;  
}

void semFree(sem_t freeSem) {
    sem[freeSem].next = semID;
    semID = freeSem;
}

void procQueueInit(struct ProcQueue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

PCB *procQueuePop(struct ProcQueue *queue) {
    PCB *popPcb = queue->procQueue[queue->head].proc;
    queue->head = (queue->head + 1) % MAX_PROCQUEUE_SIZE;
    return popPcb;
}

PCB *procQueuePush(struct ProcQueue *queue, PCB *pushPcb) {
    queue->procQueue[queue->tail].proc = pushPcb;
    queue->tail = (queue->tail + 1) % MAX_PROCQUEUE_SIZE;
    return pushPcb;
}

void P(Semaphore *s, PCB *waitPcb) {
    s->value --;
    if (s->value < 0) {
        PCB *sleepPcb = procQueuePush(&s->queue, waitPcb);
        sleepPcb->state = BLOCKED;
    }
}

void V(Semaphore *s) {
    s->value ++;
    if (s->value <= 0) {
        PCB *wakePcb = procQueuePop(&s->queue);
        Log("To wake %d", wakePcb->pid);
        wakePcb->state = RUNNABLE;
    }
}

int semCreate(sem_t *usr_sem, int value) {
    *usr_sem = semAlloc();
    sem[*usr_sem].value = value;
    procQueueInit(&sem[*usr_sem].queue);
    Log("semID: %d",*usr_sem);
    return 0;
}

int semPost(sem_t *semID) {
    V(&sem[*semID]);
    return 0;
}

int semWait(sem_t *semID, PCB *waitPcb) {
    P(&sem[*semID], waitPcb);
    return 0;
}

int semDestroy(sem_t *sem) {
    semFree(*sem);
    return 0;
}