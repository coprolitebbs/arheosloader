; =================================================
; GDT
; =================================================
BITS 16

align 8


gdt:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
	
	
	;gdt:
    ;dw 0FFFFh
    ;dw 0000h
    ;db 00h
    ;db 10010010b
    ;db 11001111b
    ;db 00h
	
gdt_end:

gdtr:
    dw gdt_end-gdt-1

    dd gdt