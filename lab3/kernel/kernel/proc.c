#include "x86.h"
#include "klib.h"
#include "device.h"
#include "debug.h"

#define MAX_NPROC 10
#define MAX_TIME 5

PCB pcb[MAX_NPROC];
static int npid = 0;
PCB *idle;
PCB *free;
PCB *L;
PCB *current;


#define MAX_APP_SIZE    0X100000
#define APP_START       0x200000
static uint8_t *app_end = (uint8_t *)APP_START;


void printList();

PCB *allocPCB();
void freePCB(PCB * recyclePCB);
void addList(PCB *proc);
PCB *removeList(int pid);
void makeProc(PCB *proc, void *entry, int type);
void createProc(void *entry, int type);

void printList(){
    PCB *cur;

    printf("idle: %d ", pcb[0].pid);

    cur = L;
    printf("L: ");
    while(cur != NULL) {
        printf("%d ", cur->pid);
        cur = cur->next;
    }

    cur = free;
    printf("free: ");
    while(cur != NULL) {
        printf("%d ", cur->pid);
        cur = cur->next;
    }
    printf("\n");
}

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

    Log("makeTrapFrame: entry: %p\n", entry); 
    push(&kstack, (uint32_t)entry);  // eip
    push(&kstack, 0);                // error
    push(&kstack, 0);               // irq

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

static void IDLE(void) {
	while(1){
		printf(".");
		waitForInterrupt();
	}
}

void initPCB() {
    current = NULL;
    L = NULL;
    free = pcb;
    for (int i = 0; i < MAX_NPROC - 1; ++i)
        pcb[i].next = &pcb[i+1];
    pcb[MAX_NPROC - 1].next = NULL;

    idle = allocPCB();
    makeProc(idle, IDLE, KTHREAD);

	createProc(loadUMain(), UPROC);
}

PCB *allocPCB() {
    if (free == NULL)
        Panic("No space for malloc!");
    PCB *newPcb = free;
    free = free->next;
    return newPcb;
}

void freePCB(PCB * recyclePCB) {
    recyclePCB->next = free;
    free = recyclePCB;
}

void addList(PCB *proc) {
    proc->next = NULL;
    
    if (L == NULL){
        L = proc;
    }
    else {
        PCB *scan = L;
        while(scan->next != NULL)
            scan = scan->next;
        scan->next = proc;
    }
    // proc->next = L;
    // L = proc;
    printList();
}

PCB *removeList(int pid) {
    assert(L != NULL);
    
    PCB *ret;

    if (L->pid == pid){
        ret = L;
        L = L->next;
        return ret;
    }
    else{
        PCB *proc = L;
        while (proc->next != NULL) {
            if (proc->next->pid == pid){
                ret = proc->next;
                proc->next = ret->next;
                return ret;
            }
            proc = proc->next;
        }
    }

    Panic("Should not reach here");
    return NULL;
}

void makeProc(PCB *proc, void *entry, int type) {
    if (type == UPROC){
        proc->base = app_end - APP_START;
        app_end += MAX_APP_SIZE;
    }
    
    proc->tf = makeTrapFrame((uint32_t *)(proc->stack + MAX_STACK_SIZE),
        (uint32_t *)app_end, entry, type);
    // print_tf(proc->tf);        
    proc->pid = npid++;
    proc->sleepTime = 0;
    proc->timeCount = MAX_TIME;
    proc->state = RUNNABLE;
    proc->type = type; 

    Log("makeProc: pid: %d", proc->pid);
} 

void createProc(void *entry, int type) {
    PCB *proc = allocPCB();
    makeProc(proc, entry, type);
    addList(proc);
}

void forkProc(PCB *fatherProc) {
    PCB *childProc = allocPCB();

    // copy kernel stack (trap frame included)
    memcpy(childProc->stack, fatherProc->stack, MAX_STACK_SIZE);

    // Remember to make proc->tf point to the new trap frame
    childProc->tf = (struct TrapFrame *)(childProc->stack + ((uint8_t *)fatherProc->tf - fatherProc->stack));

    // reset pid
    assert(childProc->pid == fatherProc->pid);
    childProc->pid = npid++;

    // reset return value
    childProc->tf->eax = 0;
    fatherProc->tf->eax = childProc->pid;

    // set base and allocate a new app space
    childProc->base = app_end - APP_START;
    app_end += MAX_APP_SIZE;

    // copy code, data and user stack
    memcpy(childProc->base + APP_START, fatherProc->base + APP_START, MAX_APP_SIZE);
    // print_tf(childProc->tf);

    addList(childProc);

    printf("fork: child kstack: %p, parent kstack: %p\n", childProc->stack, fatherProc->stack);
    printf("fork: child base: %p, parent base: %p\n", childProc->base, fatherProc->base);
    printf("fork: child pid: %p, parent pid: %p\n", childProc->pid, fatherProc->pid);
    Log("fork success!");
}


void destroyProc(PCB *proc) {
    assert(current != NULL);
    freePCB(removeList(proc->pid));
}

void sleepProc(PCB *proc, unsigned int sleepTime) {
    proc->state = BLOCKED;
    proc->sleepTime = sleepTime * HZ;
}

PCB *schedule() {
    PCB *i;
    
    if (current == idle) {
        if (L == NULL)
            return idle;
        
        i = L;
        while (i != NULL) {
            if (i->state == RUNNABLE) {
                Log("schedule: from %d to %d", current->pid, i->pid);
                return i;
            }
            i = i->next;
        }
        return idle;
    }

    assert(current != idle);
    if (current->state == RUNNABLE && current->timeCount > 0)
        return current;

    i = current->next;
    if (i == NULL)
        i = L;

    while (1) {
        if (i->state == RUNNABLE) {
            Log("schedule: from %d to %d", current->pid, i->pid);
            return i;
        }

        if (i == current) {
            Log("schedule: from %d to idle", current->pid);
            return idle;
        }

        i = i->next;
        if(i == NULL)
            i = L;
    }
    Panic("Should not reach here!");
    return idle;
}

struct TrapFrame *switchProc(PCB *proc) {
    extern TSS tss;
    extern SegDesc gdt[NR_SEGMENTS];

    current = proc;    
    tss.esp0 = (uint32_t)(current->stack + MAX_STACK_SIZE);
	tss.ss0  = KSEL(SEG_KDATA);
    if (current->type == UPROC) {
        gdt[SEG_UCODE] = SEG(STA_X | STA_R, (uint32_t)current->base, 0xffffffff, DPL_USER);
        gdt[SEG_UDATA] = SEG(STA_W,         (uint32_t)current->base, 0xffffffff, DPL_USER);
    }

    if (current->timeCount == 0)
        current->timeCount = MAX_TIME;
    proc->state = RUNNING;
    return current->tf;
}

void timeReduceProc() {
    assert(current != NULL);

    current->timeCount--;
    for (PCB *i = L; i != NULL; i = i->next) {
        if (i->state == BLOCKED) {
            i->sleepTime--;
            if (i->sleepTime == 0)
                i->state = RUNNABLE;
        }
    }
}


void empty1() {
    while(1);
}

void empty2() {
    while(1);
}

void test() {
    Log("test Log.");

	
    printList();
    
    PCB *proc1 = allocPCB();
    makeProc(proc1,empty1, UPROC);
    addList(proc1);
    
    printList();

    PCB *proc2 = allocPCB();
    makeProc(proc2,empty2, UPROC);
    addList(proc2);

    switchProc(schedule());

    printList();
    freePCB(removeList(proc1->pid));
    freePCB(removeList(proc2->pid));
    printList();

    Panic("test Panic:Stop here");
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

