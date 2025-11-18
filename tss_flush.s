; tss_flush.s
BITS 32
GLOBAL tss_flush
tss_flush:
    mov ax, [esp+4]   ; TSS selector (e.g., 0x28)
    ltr ax
    ret
