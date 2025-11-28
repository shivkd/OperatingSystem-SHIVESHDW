# Object files in the kernel
OBJECTS = loader.o kmain.o framebuffer.o io.o serial.o gdt.o gdt_flush.o \
           interrupts.o interrupts_asm.o pic.o keyboard.o \
           tss.o tss_flush.o usermode.o paging.o frame_alloc.o \
           fs.o initfs.o syscall.o syscall_asm.o proc.o pit.o

# Toolchains
CC      = gcc      # for 32-bit kernel C files (with CFLAGS)
AS      = nasm
LD      = ld
HOSTCC  = gcc      # normal host compiler for tools/mkinitfs

# C compiler flags (kernel)
CFLAGS  = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
          -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c

# Assembler flags (32-bit ELF)
ASFLAGS = -f elf32

# Linker flags
LDFLAGS = -T link.ld -melf_i386

# Default target
all: kernel.elf

# Link the kernel
kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

# Optional ISO target from the book
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

# Run the kernel directly via QEMU
run: kernel.elf
	qemu-system-i386 -kernel kernel.elf \
	                 -serial file:com1.out \
	                 -monitor stdio

# Generic: compile any .c to .o
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Generic: assemble any .s to .o
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# Explicit rule for interrupts_asm.s if you want it
interrupts_asm.o: interrupts_asm.s
	$(AS) $(ASFLAGS) $< -o $@

# -------- initfs tool + image + object --------

# Build host-side tool to pack initfs/ into initfs.img
tools/mkinitfs: tools/mkinitfs.c fs.h
	$(HOSTCC) tools/mkinitfs.c -o tools/mkinitfs

# Build the initfs image from the initfs/ directory
initfs.img: tools/mkinitfs
	tools/mkinitfs initfs initfs.img

# Convert initfs.img into an ELF object linked into the kernel
initfs.o: initfs.img
	objcopy -I binary -O elf32-i386 -B i386 initfs.img initfs.o

# Clean build products
clean:
	rm -f *.o kernel.elf os.iso initfs.img
	rm -f tools/mkinitfs
