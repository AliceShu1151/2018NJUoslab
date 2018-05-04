#include "x86.h"
#include "klib.h"
#include "debug.h"

enum{
    RUNNABLE,
    RUNNING,
    BLOCKED,
    DEAD
};

#define MAX_NPROC 4
static PCB pcb[MAX_NPROC];
static int nproc = 0;
static int npid = 1;
PCB *current = NULL;

#define MAX_APP_SIZE    0X100000
#define APP_START       0x200000
static uint8_t *app_end = (uint8_t *)APP_START;

void print_tf(struct TrapFrame *tf) {
    printf("addr: %p\n", tf);
    uint32_t *p = (uint32_t *)tf;
    for (int i = 0; i < 19; ++i) {
        printf("%x ", p[i]);
        if ((i % 4) == 3)
            printf("\n");
    }
    printf("\n");
}

static inline void push(uint32_t **pstack, uint32_t val) {
    *(--*pstack) = val;
}

struct TrapFrame *makeTrapFrame(uint32_t *kstack, uint32_t*ustack, void *entry, int type){
    if(type == UPROC){
        push(&kstack,USEL(SEG_UDATA)); //ss
        push(&kstack,(uint32_t)ustack);//esp
    }
    // eflags: IF = 1
    push(&kstack,0x202);

    if (type == UPROC)
        push(&kstack,USEL(SEG_UCODE));
    else if (type == KTHREAD)
        push(&kstack,KSEL(SEG_KCODE));
    else
        assert(0);

    printf("makeTrapFrame: entry: %p\n", entry); 
    push(&kstack, (uint32_t)entry);  // eip
    push(&kstack, 0);                // error
    push(&kstack, -1);               // irq

    // edi, esi, ebp, xxx, ebx, edx, ecx, eax;
    push(&kstack, 0); push(&kstack, 0);
    push(&kstack, 0); push(&kstack, 0);
    push(&kstack, 0); push(&kstack, 0);
    push(&kstack, 0); push(&kstack, 0);

    // gs, fs, es, ds
    if (type == UPROC) {
        push(&kstack, USEL(SEG_UDATA));
        push(&kstack, USEL(SEG_UDATA));
        push(&kstack, USEL(SEG_UDATA));
        push(&kstack, USEL(SEG_UDATA));
    } else if (type == KTHREAD) {
        push(&kstack, KSEL(SEG_KDATA));
        push(&kstack, KSEL(SEG_KDATA));
        push(&kstack, KSEL(SEG_KDATA));
        push(&kstack, KSEL(SEG_VIDEO));
    } else {
        assert(0);
    }

    return (struct TrapFrame *)kstack;
}

void makeProc(void *entry, int type){
    // #define MAX_NPROC 4
    // static PCB pcb[MAX_NPROC];
    assert(nproc < 4);
     
    int i = nproc++;

    if (type == UPROC){
        pcb[i].base = app_end - APP_START;
        app_end += MAX_APP_SIZE;
    }
    pcb[i].tf = makeTrapFrame((uint32_t *)(pcb[i].stack + MAX_STACK_SIZE),
        (uint32_t *)app_end, entry, type);
    print_tf(pcb[i].tf);
    pcb[i].pid = npid++;
    pcb[i].sleepTime = 0;
    pcb[i].timeCount = 0;
    pcb[i].state = RUNNABLE;
    pcb[i].type = type; 
} 

void forkProc(PCB *proc){
    int i = nproc++;

    // copy kernel stack (trap frame included)
    memcpy(pcb[i].stack, proc->stack, MAX_STACK_SIZE);

    // Remember to make pcb[i].tf point to the new trap frame
    pcb[i].tf = (struct TrapFrame *)(pcb[i].stack + ((uint8_t *)pcb[i].tf - proc->stack));

    // reset pid
    assert(pcb[i].pid == proc->pid);
    pcb[i].pid = npid++;

    // reset return value
    pcb[i].tf->eax = 0;
	proc->tf->eax = proc->pid;

    // set base and allocate a new app space
    pcb[i].base = app_end - APP_START;
    app_end += MAX_APP_SIZE;

    // copy code, data and user stack
    memcpy(pcb[i].base + APP_START, proc->base + APP_START, MAX_APP_SIZE);

    printf("fork: child kstack: %p, parent kstack: %p\n", pcb[i].stack, proc->stack);
    printf("fork: child base: %p, parent base: %p\n", pcb[i].base, proc->base);
    printf("fork: child pid: %p, parent pid: %p\n", pcb[i].pid, proc->pid);
    Log("fork success!");
}

void killProc(PCB *proc){
    proc->state = DEAD;
}

PCB *schedule() {
    int i;

    // TODO: need a more complicated schedule scheme 
    for (i = nproc - 1; i > 0; --i) {
        if (pcb[i].state == RUNNABLE) {
            printf("schedule: schedule to pcb[%d]\n", i);
            return &pcb[i];
        }
    }

    printf("schedule: You will enter to IDLE, but I abort it!\n");
    assert(0);  // delete this line to enable switch to IDLE thread
    return &pcb[0];
}

struct TrapFrame *switchProc(PCB *proc) {
    assert (proc != NULL);
    extern TSS tss;
    extern SegDesc gdt[NR_SEGMENTS];

    if (current == proc)
        return current->tf;
    
    current = proc;
    tss.esp0 = (uint32_t)(current->stack + MAX_STACK_SIZE);
	tss.ss0  = KSEL(SEG_KDATA);
    if (current->type == UPROC) {
        gdt[SEG_UCODE] = SEG(STA_X | STA_R, (uint32_t)current->base, 0xffffffff, DPL_USER);
        gdt[SEG_UDATA] = SEG(STA_W,         (uint32_t)current->base, 0xffffffff, DPL_USER);
    }
    return current->tf;
}