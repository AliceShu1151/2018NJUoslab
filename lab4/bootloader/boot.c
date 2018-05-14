#include "boot.h"

#define SECTSIZE 512

void bootMain(void) {
	/* 加载内核至内存，并跳转执行 */
	char *buf, *dst, *src;
	struct ELFHeader *elf;
	struct ProgramHeader *ph;
	void (*entry)(void);
	int i, j;

	/* load kernel code*/
	buf = (char *)0x5000000;
	for (i = 1; i <= 200; i++)
		readSect((void*)(buf + 512 * (i - 1)), i);

	elf = (struct ELFHeader *)buf;
	ph = (struct ProgramHeader *)(buf + elf->phoff);

	for (i = 0; i < elf->phnum; i++,ph++)
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
	}

	/* jump to kernel */
	//((void (*)(void))elf->entry)();
	entry = (void(*)(void))(elf->entry);
	entry();
}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading a sector of disk
	int i;
	waitDisk();
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}
