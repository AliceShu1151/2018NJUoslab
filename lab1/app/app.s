.code32

.global start
start:                       
	movl $((80*9+0)*2), %edi                #在第9行第0列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x48, %al                         #48为H的ASCII码
	movw %ax, %gs:(%edi)                    #写显存 
    
	movl $((80*9+1)*2), %edi                #在第9行第1列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x65, %al                         #65为e的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+2)*2), %edi                #在第9行第2列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x6c, %al                         #6c为l的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+3)*2), %edi                #在第9行第3列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x6c, %al                         #6c为l的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+4)*2), %edi                #在第9行第4列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x6f, %al                         #6f为o的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+5)*2), %edi                #在第9行第5列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x2c, %al                         #2c为,的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+6)*2), %edi                #在第9行第6列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x20, %al                         #20为 的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+7)*2), %edi                #在第9行第7列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x57, %al                         #57为W的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+8)*2), %edi                #在第9行第8列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x6f, %al                         #6f为o的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+9)*2), %edi                #在第9行第9列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x72, %al                         #72为r的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+10)*2), %edi               #在第9行第10列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x6c, %al                         #6c为l的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+11)*2), %edi               #在第9行第11列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x64, %al                         #64为d的ASCII码
	movw %ax, %gs:(%edi)                    #写显存

	movl $((80*9+12)*2), %edi               #在第9行第12列打印
	movb $0x0c, %ah                         #黑底红字
	movb $0x21, %al                         #21为!的ASCII码
	movw %ax, %gs:(%edi)                    #写显存
  
loop:  
	jmp loop
