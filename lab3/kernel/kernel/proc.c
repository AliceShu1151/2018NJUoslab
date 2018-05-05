#include "x86.h"
#include "klib.h"
#include "debug.h"



PCB pcb[MAX_NPROC];
static int nproc = 0;
static int npid = 1;
int next[MAX_NPROC];
static int free;
int L;
// PCB *current = NULL;

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

int newPCB(){
    int newProc;
    newProc = free;
    free = next[free];
    return newProc;
}

int getPCB(){
    if (free == NILL){
        Log("NO space for new pcb");
        assert(0);
    }

    int newProc = newPCB();

    next[newProc] = L;
    L = newProc;

    Log("You get a new pcb block!");
    return newProc;
}

void printFreeL(){
    int i;
    
    i = L;
    printf("\nL:");
    while (i != NILL){
        printf("%d ",i);
        i = next[i];
    }

    printf("\nfree:");
    i = free;
    while (i != NILL){
        printf("%d ",i);
        i = next[i];
    }
    printf("\n");
}

void initPCB(){
    int i;
    current = NILL;

    free = 0;
    L = NILL;
    for (i = 0; i < MAX_NPROC - 1; ++i)
        next[i] = i + 1;
    next[MAX_NPROC - 1] = NILL;
    printFreeL();
}

void makeProc(void *entry, int type){
    assert(nproc < MAX_NPROC);

    int i = getPCB();
    
    printFreeL();
    // Log("\ncurrent:%d",current);

    if (type == UPROC){
        pcb[i].base = app_end - APP_START;
        app_end += MAX_APP_SIZE;
    }
    
    pcb[i].tf = makeTrapFrame((uint32_t *)(pcb[i].stack + MAX_STACK_SIZE),
        (uint32_t *)app_end, entry, type);
    print_tf(pcb[i].tf);        
    pcb[i].pid = npid++;
    pcb[i].sleepTime = 0;
    pcb[i].timeCount = MAX_TIME;
    pcb[i].state = RUNNABLE;
    pcb[i].type = type; 
} 

void forkProc(PCB *proc){
    int i = getPCB();

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
    print_tf(pcb[i].tf);

    printf("fork: child kstack: %p, parent kstack: %p\n", pcb[i].stack, proc->stack);
    printf("fork: child base: %p, parent base: %p\n", pcb[i].base, proc->base);
    printf("fork: child pid: %p, parent pid: %p\n", pcb[i].pid, proc->pid);
    Log("fork success!");
}


void killProc(int proc){
    assert(current != NILL);
    pcb[proc].state = DEAD;

    // int  i = L;

    // while(next[i] != current);

    // pcb[current].state = DEAD;
    // next[i] = next[current];
    // next[current] = free;
    // free = current;
}

void sleepProc(PCB *proc, unsigned int  sleepTime){
    proc->state = BLOCKED;
    proc->sleepTime = sleepTime * HZ;
}

int schedule() {
    if (pcb[current].state == RUNNABLE && pcb[current].timeCount > 0)
        return current;

    int i = next[current];
    if(i == NILL)
        i = L;
    
    while (1) {
        if (pcb[i].state == RUNNABLE) {
            Log("schedule: from %d to %d", current, i);
            return i;
        }
        if (i == current)
            break;
        i = next[i];
        if(i == NILL)
            i = L;
    }

    Panic("Should not reach here!");
    return 0;
}

struct TrapFrame *switchProc(int proc) {
    assert (proc != NILL);
    extern TSS tss;
    extern SegDesc gdt[NR_SEGMENTS];

    current = proc;    
    tss.esp0 = (uint32_t)(pcb[current].stack + MAX_STACK_SIZE);
	tss.ss0  = KSEL(SEG_KDATA);
    if (pcb[current].type == UPROC) {
        gdt[SEG_UCODE] = SEG(STA_X | STA_R, (uint32_t)pcb[current].base, 0xffffffff, DPL_USER);
        gdt[SEG_UDATA] = SEG(STA_W,         (uint32_t)pcb[current].base, 0xffffffff, DPL_USER);
    }

    if (pcb[current].timeCount == 0)
        pcb[current].timeCount = MAX_TIME;
    pcb[proc].state = RUNNING;
    return pcb[current].tf;
}

void timeReduceProc() {
	int i;

    pcb[current].timeCount--;

    for (i = L; i != NILL; i = next[i]) {
        printf("pcb[%d] sleepTime: %d\n",i,pcb[current].sleepTime); 
        printf("pcb[%d] state: %d\n",i,pcb[current].state); 
        if (pcb[i].state == BLOCKED) {
            pcb[i].sleepTime--;
            if (pcb[i].sleepTime == 0)
                pcb[i].state = RUNNABLE;
        }
         printf("pcb[%d] sleepTime: %d\n",i,pcb[current].sleepTime);  
    }

    // printf("current timeCount: %d\n",pcb[current].timeCount);
    
}

// PCB *idle;

// PCB *allocPCB();
// void freePCB(PCB *);
// void initPCB() {
//     // init list
//     // init free
//     idle = allocPCB();
//     makeProc(proc, IDLE, type);
// }

// void addList(PCB *);
// void removeList(PCB *);

// void makeProc(PCB *proc, void *entry, int type);

// void createProc(void *entry, int type) {
//     PCB *proc = allocPCB();
//     makeProc(proc, entry, type);
//     addList(proc);
// }

