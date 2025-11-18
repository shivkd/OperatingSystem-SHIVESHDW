global gdt_flush

; void gdt_flush(unsigned int gp_address);
; stack layout on entry:
;   [esp]     = return address
;   [esp + 4] = gp_address (pointer to struct gdt_ptr)

gdt_flush:
    ; load pointer to gdt_ptr into EAX
    mov eax, [esp + 4]

    ; load GDTR with our GDT descriptor
    lgdt [eax]

    ; reload data segment registers with kernel data segment selector (index 2 -> 0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; far jump to reload CS with kernel code segment selector (index 1 -> 0x08)
    jmp 0x08:flush_done

flush_done:
    ret
