global outb ; make the label visible outside the file

;outb send ab yte to an ioport
; stack esp + 8 data bytes
; esp + 4 the io port
; esp return addres
outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al; send al to dx
    ret

global inb
; inb - returns a byte from the given I/O port
; stack: [esp + 4] The address of the I/O port
;        [esp    ] The return address
inb:
    mov dx, [esp + 4]    ; port -> DX
    in  al, dx           ; read from port into AL
    ret                  ; return AL
