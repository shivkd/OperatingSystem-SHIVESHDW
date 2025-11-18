# Object files in the kernel
OBJECTS = loader.o kmain.o framebuffer.o io.o serial.o gdt.o gdt_flush.o \
           interrupts.o interrupts_asm.o pic.o keyboard.o \
           tss.o tss_flush.o usermode.o



# Tools
CC      = gcc
AS      = nasm
LD      = ld

# C compiler flags (book version)
CFLAGS  = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
          -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c

# Assembler flags (32-bit ELF)
ASFLAGS = -f elf32

# Linker flags (same linker script as chapter 2)
LDFLAGS = -T link.ld -melf_i386

# Default target
all: kernel.elf

# Link the kernel
kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

# Build the ISO (optional, matches book; you already have iso/ set up)
os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R                              \
	            -b boot/grub/stage2_eltorito    \
	            -no-emul-boot                   \
	            -boot-load-size 4               \
	            -A os                           \
	            -input-charset utf8             \
	            -quiet                          \
	            -boot-info-table                \
	            -o os.iso                       \
	            iso

# Run the kernel directly via QEMU (simpler than BIOS+GRUB on WSL)
run: kernel.elf
	qemu-system-i386 -kernel kernel.elf \
	                 -serial file:com1.out \
	                 -monitor stdio


# Pattern rule: compile any .c to .o
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Pattern rule: assemble any .s to .o
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

ASFLAGS = -f elf32   # or -f elf, just stay consistent with your other .s

interrupts_asm.o: interrupts_asm.s
	$(AS) $(ASFLAGS) $< -o $@


# Clean build products
clean:
	rm -f *.o kernel.elf os.iso
