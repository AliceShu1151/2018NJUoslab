#include "common.h"
#include "x86.h"
#include "device.h"
#include "klib.h"
#include "debug.h"


void kEntry(void) {
	initSerial();// initialize serial port
	initTimer();
	initIdt(); // initialize idt
	initIntr(); // iniialize 8259a
	initSeg(); // initialize gdt, tss
	initPCB();

	//test();

	enableInterrupt();
	while(1);
	assert(0);
}
