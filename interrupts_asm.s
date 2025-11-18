; nasm macros to create per interrupt stubs
; common interrupts handler that normalizes stack, save registers, calls c function interrupt_handler)_
;restores registers and stack
bits 32
extern interrupt_handler
global load_idt

;macros for intterupts that d onot pus an eorro code
%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0 ; fake error code
    push dword %1 ; interrupt number
    jmp common_interrupt_handler
%endmacro

; Macro for interrupts that DO push an error code.
%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1         ; interrupt number
    jmp  common_interrupt_handler
%endmacro

common_interrupt_handler:
    ; [esp]   = int_num
    ; [esp+4] = errcode (real or 0)
    ; then CPU-pushed frame (eflags, cs, eip) or (errcode, eflags, cs, eip)

    pushad
    ; after pushad, int_num is at [esp+32]
    push dword [esp + 32]
    call interrupt_handler
    add  esp, 4
    popad

    add  esp, 8        ; <-- drop int_num + errcode (real or fake)
    iret               ; returns to saved eip/cs/eflags




; === CPU exceptions ===
no_error_code_interrupt_handler 0
no_error_code_interrupt_handler 1
no_error_code_interrupt_handler 2
no_error_code_interrupt_handler 3
no_error_code_interrupt_handler 4
no_error_code_interrupt_handler 5
no_error_code_interrupt_handler 6
no_error_code_interrupt_handler 7
error_code_interrupt_handler    8
no_error_code_interrupt_handler 9
error_code_interrupt_handler   10
error_code_interrupt_handler   11
error_code_interrupt_handler   12
error_code_interrupt_handler   13
error_code_interrupt_handler   14
no_error_code_interrupt_handler 15
no_error_code_interrupt_handler 16
error_code_interrupt_handler   17
no_error_code_interrupt_handler 18
no_error_code_interrupt_handler 19
no_error_code_interrupt_handler 20
no_error_code_interrupt_handler 21
no_error_code_interrupt_handler 22
no_error_code_interrupt_handler 23
no_error_code_interrupt_handler 24
no_error_code_interrupt_handler 25
no_error_code_interrupt_handler 26
no_error_code_interrupt_handler 27
no_error_code_interrupt_handler 28
no_error_code_interrupt_handler 29
no_error_code_interrupt_handler 30
no_error_code_interrupt_handler 31

; === IRQs after PIC remap: 0x20–0x2F ===
no_error_code_interrupt_handler 32
no_error_code_interrupt_handler 33
no_error_code_interrupt_handler 34
no_error_code_interrupt_handler 35
no_error_code_interrupt_handler 36
no_error_code_interrupt_handler 37
no_error_code_interrupt_handler 38
no_error_code_interrupt_handler 39
no_error_code_interrupt_handler 40
no_error_code_interrupt_handler 41
no_error_code_interrupt_handler 42
no_error_code_interrupt_handler 43
no_error_code_interrupt_handler 44
no_error_code_interrupt_handler 45
no_error_code_interrupt_handler 46
no_error_code_interrupt_handler 47

; === IDT loader ===
; void load_idt(unsigned int idt_ptr_address);
load_idt:
    mov eax, [esp + 4]    ; eax = &idt_desc
    lidt [eax]            ; load IDT register
    ret
