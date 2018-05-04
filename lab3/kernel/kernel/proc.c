#include "x86.h"
#include "klib.h"
#include "debug.h"

enum{
    RUNNABLE,
    RUNNING,
    BLOCKED,
    DEAD
};


PCB pcb[MAX_NPROC];
static int nproc = 0;
static int npid = 1;
static int next[MAX_NPROC];
static int free;
static int L;
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

int getPCBBlock(){
    int newProc;
    
    if (free == -1){
        Log("NO space for new pcb");
        assert(0);
    }

    newProc = free;
    free = next[free];

    next[newProc] = L;
    L = newProc;

    Log("You get a new pcb block!");
    return newProc;
}

void print_List(){
    int i;
    
    i = L;
    printf("\nL:");
    while (i != -1){
        printf("%d ",i);
        i = next[i];
    }

    printf("\nfree:");
    i = free;
    while (i != -1){
        printf("%d ",i);
        i = next[i];
    }
    printf("\n");
}

void initPCBLink(){
    int i;

    free = 0;
    L = -1;
    for (i = 0; i < MAX_NPROC - 1; ++i)
        next[i] = i + 1;
    next[MAX_NPROC - 1] = -1;
    print_List();
}

void makeProc(void *entry, int type){
    // #define MAX_NPROC 4
    // static PCB pcb[MAX_NPROC];
    assert(nproc < MAX_NPROC);

    int i = getPCBBlock();
    
    print_List();
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
    int i = getPCBBlock();

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
    assert(current != -1);
    int  i = L;

    while(next[i] != current);

    pcb[current].state = DEAD;
    next[i] = next[current];
    next[current] = free;
    free = current;
}

void sleepProc(PCB *proc, unsigned int  sleepTime){
    proc->state = BLOCKED;
    proc->sleepTime = sleepTime * HZ;
}

int schedule() {
    int i;
    i = next[current];
    if(i == -1)
        i = L;
    
    pcb[current].state = RUNNABLE;
    
    print_List();
    printf("timeCount: %d\n",pcb[current].timeCount);

    //if (pcb[current].timeCount == 0){
        while (i != current){
            Log("the link:");
            print_List();
            if(i == -1)
                i = L;
            Log("schedule  i: %d    current: %d\n",i,current);
            printf("The pcb[i]:\n");
            print_tf(pcb[i].tf);
            if (pcb[i].state == RUNNABLE) {
                current = i;
                pcb[current].timeCount = MAX_TIME;
                Log("schedule: schedule to pcb[%d]\n", i);
                // print_tf(pcb[i].tf);
                return i;
            }
            else if (pcb[i].state == BLOCKED){
                if (pcb[i].sleepTime == 0)
                    pcb[i].state = RUNNABLE;
                Log("schdule: pcb[%d] wake up!\n", i);
            }
            i = next[i];
        } 
    //}
    return current;
}

struct TrapFrame *switchProc(int proc) {
    assert (proc != -1);
    extern TSS tss;
    extern SegDesc gdt[NR_SEGMENTS];

    pcb[proc].state = RUNNING;
    // Log("proc  i: %d    current: %d\n",proc,current);
    if (current == proc)
        return pcb[current].tf;
    
    current = proc;
    tss.esp0 = (uint32_t)(pcb[current].stack + MAX_STACK_SIZE);
	tss.ss0  = KSEL(SEG_KDATA);
    if (pcb[current].type == UPROC) {
        gdt[SEG_UCODE] = SEG(STA_X | STA_R, (uint32_t)pcb[current].base, 0xffffffff, DPL_USER);
        gdt[SEG_UDATA] = SEG(STA_W,         (uint32_t)pcb[current].base, 0xffffffff, DPL_USER);
    }
    return pcb[current].tf;
}

void timeReduceProc()
{
	int i;
    for (i = L; i != -1; i = next[i])
    {
        if (pcb[i].state == BLOCKED)
            pcb[i].sleepTime--;
        if (pcb[i].sleepTime == 0)
            pcb[i].state = RUNNABLE;
    }

    pcb[current].timeCount--;
    if (pcb[current].sleepTime == 0)
        pcb[current].state = RUNNABLE;
}