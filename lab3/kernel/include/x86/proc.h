#ifndef __PROC_H__
#define __PROC_H__

#define PGSIZE 4096
#define PG_ALIGN __attribute((aligned(PGSIZE)))
#define MAX_STACK_SIZE (4 * PGSIZE)

struct TrapFrame;

enum {
    KTHREAD, UPROC
};

typedef union ProcessTable{
    uint8_t stack[MAX_STACK_SIZE] PG_ALIGN;
    struct{
        struct TrapFrame *tf;
        int state;
        int timeCount;
        int sleepTime;
        uint32_t pid;
        uint8_t *base;
        int type;
    };
} PCB;

#define MAX_NPROC 4
#define MAX_TIME 20
#define HZ 100
extern PCB pcb[MAX_NPROC];
int current;

void initPCBLink();
int schedule();
struct TrapFrame *switchProc(int proc);
void makeProc(void *entry, int type);
void timeReduceProc();
void forkProc(PCB *proc);
void killProc(int proc);
void sleepProc(PCB *proc, unsigned int sleepTime);
#endif