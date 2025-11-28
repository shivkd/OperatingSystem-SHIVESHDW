; syscall_asm.s - entry stub for int 0x80 syscalls
BITS 32

global syscall_entry
extern syscall_dispatch
; esp + = return EIP (USER), esp +4 = return cs (use), esp+8 = eflags
;esp+12 = ueser es[ , user ss]
syscall_entry:
    ; save general purpose registers
    pushad
    ;now esp points to saved registers layout matches struct reg
    mov eax, esp ; eax = &regs
    push eax ; arg to syscall
    call syscall_dispatch
    add esp, 4 ; pop arg
    ; C returned value in eax
    ; overwrite saved eax in regs at 28 from esp
    mov [esp + 28], eax
    ;restore registers (eax no contains syscall return value)
    popad
    ; return to user mode using cpu save frma 
    iret
    
