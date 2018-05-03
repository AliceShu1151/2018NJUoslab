.code16

.global start
start:
	cli                             #关闭中断
        inb $0x92, %al                  #启动A20总线
        orb $0x02, %al
        outb %al, $0x92
        data32 addr32 lgdt gdtDesc      #加载GDTR
        movl %cr0, %eax                 #启动保护模式
        orb $0x01, %al
        movl %eax, %cr0			#置位PE，使能保护模式
        data32 ljmp $0x08, $start32     #长跳转切换至保护模式

.code32
start32:
        jmp bootMain                    #跳转至bootMain函数 定义于boot.c

.p2align 2
gdt: # 8 bytes for each table entry, at least 1 entry
	.word 0,0 # empty entry
	.byte 0,0,0,0

	.word 0xffff,0                  # kernel code descriptor
	.byte 0,0x9a,0xcf,0

gdtDesc: # 6 bytes in total
	.word (gdtDesc - gdt -1)
	.long gdt
