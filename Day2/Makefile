ipl.bin : ipl.asm Makefile
	nasm ipl.asm -o ipl.bin -l ipl.lst
PandOS.img : ipl.bin Makefile
	cat ipl.bin > PandOS.img

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

src_only :
	make  clean
	rm PandOS.img
