; interrupts_asm.s – 32-bit stubs for IDT entries 0–47 and load_idt
BITS 32

; ------------------------------------------------------------------
; lidt helper called from C:
;   void load_idt(unsigned int idt_ptr_address);
; C pushes the argument on the stack, so at entry:
;   [esp+4] = pointer to struct idt_ptr
; ------------------------------------------------------------------
global load_idt
load_idt:
    mov eax, [esp + 4]    ; eax = &idt_desc
    lidt [eax]            ; load IDT from memory at that address
    ret

; ------------------------------------------------------------------
; Common ISR stubs: build struct regs on the stack and call
;   void interrupt_handler(struct regs *r, unsigned int interrupt);
; ------------------------------------------------------------------
extern interrupt_handler

%macro ISR_NOERR 1
global interrupt_handler_%1
interrupt_handler_%1:
    ; Stack on entry (from ring3, no error code):
    ;   [EIP, CS, EFLAGS, ESP, SS]

    pushad                 ; save general-purpose registers

    mov eax, esp           ; eax = struct regs* (points at EDI)

    ; C calling convention: arguments are pushed right-to-left.
    ; interrupt_handler(struct regs *r, unsigned int interrupt):
    ;   push interrupt
    ;   push r
    push dword %1          ; 2nd arg: interrupt number
    push eax               ; 1st arg: pointer to regs
    call interrupt_handler
    add esp, 8             ; pop both arguments

    popad                  ; restore registers (possibly updated via *r)
    iretd                  ; return using hardware frame (EIP, CS, ...)
%endmacro

; CPU exceptions 0–31
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_NOERR 8
ISR_NOERR 9
ISR_NOERR 10
ISR_NOERR 11
ISR_NOERR 12
ISR_NOERR 13
ISR_NOERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; IRQs 0–15 (32–47)
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 35
ISR_NOERR 36
ISR_NOERR 37
ISR_NOERR 38
ISR_NOERR 39
ISR_NOERR 40
ISR_NOERR 41
ISR_NOERR 42
ISR_NOERR 43
ISR_NOERR 44
ISR_NOERR 45
ISR_NOERR 46
ISR_NOERR 47
