#include "x86.h"
#include "device.h"
#define	SYS_write	1 
void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */

	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}
}

void sys_write(struct TrapFrame *tf)
{
	asm volatile("movl %0, %%eax":: "r"(KSEL(SEG_VIDEO)));
	asm volatile("movw %ax, %gs");	
	static int row = 0, col = 0;
	char c = '\0';
	/* ebx:str type  ecx:str  edx:length */
	if (tf->ebx == 1 || tf->ebx == 2) {
		int i;
		for(i = 0; i < tf->edx; i++) {
			c = *(char *)(tf->ecx + i);
			putChar(c);
			if (c == '\n') {
				row++;
				col = 0;
				continue;
			}
			if (col == 80) {
				row++;
				col = 0;
			}
			video_print(row, col++, c);
		}
		tf->eax = tf->edx;
	}
}


void syscallHandle(struct TrapFrame *tf) {
	/* 实现系统调用*/
	switch(tf->eax) {
		case SYS_write: sys_write(tf); break;
		/* we can add more syscall */
		default: assert(0);
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
