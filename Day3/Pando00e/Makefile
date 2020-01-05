os_name=PandOS.img

ipl.bin : ipl.asm Makefile
	nasm ipl.asm -o ipl.bin -l ipl.lst

PandOS.sys: PandOS.asm Makefile
	nasm PandOS.asm -o PandOS.sys -l PandOS.lst

PandOS.img : ipl.bin PandOS.sys Makefile
	mformat -f 1440 -C -B ipl.bin -i ${os_name} ::
	mcopy -i ${os_name} PandOS.sys ::

asm:
	make -r ipl.bin
img:
	make -r PandOS.img

run:
	make img
	qemu-system-i386 PandOS.img

clean :
	rm ipl.bin
	rm ipl.lst
	rm PandOS.sys
	rm PandOS.lst

src_only :
	make  clean
	rm PandOS.img
