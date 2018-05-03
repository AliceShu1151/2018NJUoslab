#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];
TSS tss;

#define SECTSIZE 512

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
	int i;
	waitDisk();
	
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);
;
	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}

void initSeg() {
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	gdt[SEG_VIDEO] = SEG(STA_W,  0x0b8000,       0xffffffff, DPL_KERN);
	setGdt(gdt, sizeof(gdt));

	/* init TSS */
	tss.esp0 = 0x200000;   // set kernel esp to 0x200,000
	tss.ss0  = KSEL(SEG_KDATA);
	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/* set segment register */
	asm volatile("movl %0, %%eax":: "r"(KSEL(SEG_KDATA)));
	asm volatile("movw %ax, %ds");
	asm volatile("movw %ax, %es");
	asm volatile("movw %ax, %ss");
	asm volatile("movw %ax, %fs");
	asm volatile("movl %0, %%eax":: "r"(KSEL(SEG_VIDEO)));
	asm volatile("movw %ax, %gs");

	/*设置正确的段寄存器*/

	lLdt(0);
	
}

void enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * and use 'iret' to jump to ring3
	 */
	asm volatile("pushl %0":: "r"(USEL(SEG_UDATA)));	// %ss
	asm volatile("pushl %0":: "r"(128 << 20));		// %esp 128MB
	asm volatile("pushfl");					// %eflags
	asm volatile("pushl %0":: "r"(USEL(SEG_UCODE)));	// %cs
	asm volatile("pushl %0":: "r"(entry));		
	asm volatile("iret"); // return to user space	
}

void loadUMain(void) {

	/*加载用户程序至内存*/
	/* load main code*/
	char *buf, *dst, *src;
	struct ELFHeader *elf;
	struct ProgramHeader *ph;
	int i, j;

	/* load kernel code*/
	buf = (char *)0x6000000;
	for (i = 1; i <= 100; i++)
		readSect((void*)(buf + 512 * (i - 1)), 200 + i);

	elf = (struct ELFHeader *)buf;
	ph = (struct ProgramHeader *)(buf + elf->phoff);

	for (i = 0; i < elf->phnum; i++)
	{
		/* laod each program segment to their virtual address
		 * type=1 : LOAD section */
		if(ph->type == 1)	
		{
			/* the location of the segment is mapped to the virtual address space
			 * (for the segment of the PT_LOAD type*/
			dst = (char *)ph->vaddr;
			src = buf + ph->off;

			for (j = 0; j < ph->filesz; j++)	
				dst[j] = src[j];
			for (; j < ph->memsz; j++)
				dst[j] = 0;
		}
		ph++;
	}
	/* enter user space */
	enterUserSpace(elf->entry);
}
