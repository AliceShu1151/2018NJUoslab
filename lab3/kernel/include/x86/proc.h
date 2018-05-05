#ifndef __PROC_H__
#define __PROC_H__

#define PGSIZE 4096
#define PG_ALIGN __attribute((aligned(PGSIZE)))
#define MAX_STACK_SIZE (4 * PGSIZE)

struct TrapFrame;

enum {
    KTHREAD, UPROC
};

enum{
    RUNNABLE,
    RUNNING,
    BLOCKED,
    DEAD
};

typedef union ProcessTableBlock{
    uint8_t stack[MAX_STACK_SIZE] PG_ALIGN;
    struct{
        struct TrapFrame *tf;
        int state;
        int timeCount;
        int sleepTime;
        uint32_t pid;
        uint8_t *base;
        int type;
        union ProcessTableBlock *next;
    };
} PCB;

PCB *current;
PCB *idle;

void initPCB();
void createProc(void *entry, int type);
void timeReduceProc();
void destroyProc(PCB *proc);
void forkProc(PCB *proc);
void sleepProc(PCB *proc, unsigned int sleepTime);
PCB *schedule();
struct TrapFrame *switchProc(PCB *proc);
void test();

#endif