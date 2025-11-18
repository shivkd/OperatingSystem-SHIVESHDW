global loader                   ; the entry symbol for ELF
extern kmain

MAGIC_NUMBER equ 0x1BADB002     ; define the magic number constant
FLAGS        equ 0x0            ; multiboot flags
CHECKSUM     equ -MAGIC_NUMBER  ; calculate the checksum
                                ; (magic number + checksum + flags should equal 0)

section .text                   ; ✅ no colon here
align 4                         ; the code must be 4 byte aligned
    dd MAGIC_NUMBER             ; write the magic number to the machine code,
    dd FLAGS                    ; the flags,
    dd CHECKSUM                 ; and the checksum

loader:                         ; the loader label (defined as entry point in linker script)
    mov esp, kernel_stack + KERNEL_STACK_SIZE ; point esp to then end of the stack
    mov eax, 0xCAFEBABE         ; place the number 0xCAFEBABE in the register eax
    call kmain
.loop:
    jmp .loop                   ; loop forever

KERNEL_STACK_SIZE equ 4096        ; 4 KiB stack 

section .bss
align 4                           ; align stack on 4-byte boundary 
kernel_stack:                     ; start of stack memory
    resb KERNEL_STACK_SIZE        ; reserve 4 KiB, uninitialized (.bss)