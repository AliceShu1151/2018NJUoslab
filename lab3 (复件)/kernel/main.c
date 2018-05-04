#include "common.h"
#include "x86.h"
#include "device.h"
#include "klib.h"
#include "debug.h"


void IDLE(void){
	while(1){
		printf("Hello from IDLE\n");
		waitForInterrupt();
	}
}

void kEntry(void) {
	initSerial();// initialize serial port
	initTimer();
	initIdt(); // initialize idt
	initIntr(); // iniialize 8259a
	initSeg(); // initialize gdt, tss
	Log("create pcb for kenel");
	makeProc((void *)IDLE, KTHREAD);
	Log("create pcb for user");
	makeProc(loadUMain(), UPROC);

	enableInterrupt();
	while(1);
	assert(0);
}
