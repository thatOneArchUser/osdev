C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c lib/*.c kernel/core/*.c)
HEADERS = $(wildcard kernel/*.h  drivers/*.h cpu/*.h lib/*.h kernel/core/*.h)
OBJ_FILES = ${C_SOURCES:.c=.o cpu/interrupt.o}

build: os-image.bin
	$(RM) kernel.bin
	$(RM) *.o *.dis *.elf
	$(RM) kernel/*.o
	$(RM) boot/*.o boot/*.bin
	$(RM) drivers/*.o
	$(RM) cpu/*.o
	$(RM) lib/*.o
	$(RM) kernel/core/*.o

kernel.bin: boot/kernel_entry.o ${OBJ_FILES}
	x86_64-elf-ld -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

os-image.bin: boot/mbr.bin kernel.bin
	cat $^ > $@

run:
ifeq ($(shell test -e os-image.bin && echo -n yes),yes)
	qemu-system-i386 -fda os-image.bin -device isa-debug-exit,iobase=0xf4,iosize=0x04
else
	$(MAKE) build 
	qemu-system-i386 -fda os-image.bin -device isa-debug-exit,iobase=0xf4,iosize=0x04
endif

echo: os-image.bin
	xxd $<

kernel.elf: boot/kernel_entry.o ${OBJ_FILES}
	x86_64-elf-ld -m elf_i386 -o $@ -Ttext 0x1000 $^

debug: os-image.bin kernel.elf
	qemu-system-i386 -s -S -fda os-image.bin -d guest_errors,int &
	i386-elf-gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

%.o: %.c ${HEADERS}
	x86_64-elf-gcc -g -m32 -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

%.dis: %.bin
	ndisasm -b 32 $< > $@

clean:
	$(RM) kernel.bin
	$(RM) *.o *.dis *.elf
	$(RM) kernel/*.o
	$(RM) boot/*.o boot/*.bin
	$(RM) drivers/*.o
	$(RM) cpu/*.o
	$(RM) lib/*.o
	$(RM) kernel/core/*.o
