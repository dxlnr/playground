boot:
	@# Building
	@arm-linux-gnueabi-as -mcpu=arm926ej-s -g start.s -o start.o
	@arm-linux-gnueabi-gcc -c -mcpu=arm926ej-s -g uart.c -o uart.o
	@# Linking
	@arm-linux-gnueabi-ld -T mem.lds uart.o start.o -o boot.elf
	@arm-linux-gnueabi-objcopy -O binary boot.elf boot.bin
	@# Running
	@qemu-system-arm -M versatilepb -m 128M -audiodev id=none,driver=none -nographic -kernel boot.bin

clean:
	@rm -f *.o *.elf *.bin
