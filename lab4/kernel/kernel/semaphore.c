#include "x86.h"
#include "klib.h"
#include "device.h"
#include "debug.h"

#define MAX_NSEMA 10

void sleep(Semaphore *s);
void wakeup(Semaphore *s);

void sleep(Semaphore *s) {

}

void wakeup(Semaphore *s) {

}

void P(Semaphore *s) {
    s->value --;
    if (s->value < 0)
        sleep(s);
}

void V(Semaphore *s) {
    s->value ++;
    if (s->value <= 0)
        wakeup(s);
}

int sem_init(sem_t *sem, uint32_t value) {
    return 0;
}

int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_destroy(sem_t *sem);