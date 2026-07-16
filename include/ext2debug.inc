BITS 16
; ============================================================
; print_string
;
; DS:SI -> string
;
; ============================================================
print_string:
    push ax
    push si
	
.next:
    lodsb
    test al,al
    jz .done
    mov ah,0x0E
    int 10h
    jmp .next

.done:
    pop si
    pop ax
    ret
	
; ============================================================
; print_debug
;
; DS:SI -> string
;
; port E9
;
; ============================================================

print_debug:
    push ax
    push dx
    push si
    mov dx,0xE9
	
.loop:
    lodsb
    test al,al
    jz .done
    out dx,al
    jmp .loop
	
.done:
    pop si
    pop dx
    pop ax
    ret
	
; ============================================================
; debug_hex_byte
;
; AL
;
; ============================================================
debug_hex_byte:
    push ax
    push bx
    push dx
    mov bl,al
    shr al,4
    call debug_hex_nibble
    mov al,bl
    and al,0x0F
    call debug_hex_nibble
    pop dx
    pop bx
    pop ax
    ret
; ============================================================
; nibble
;
; AL = 0..15
;
; ============================================================
debug_hex_nibble:
    push dx
    and al,0x0F
    cmp al,9
    jbe .digit
    add al,'A'-10
    jmp .out
	
.digit:
    add al,'0'
	
.out:
    mov dx,0xE9
    out dx,al
    pop dx
    ret

; ============================================================
; word
;
; AX
;
; ============================================================
debug_hex_word:
    push ax
    push bx
    push dx
    mov bx,ax
    mov al,bh
    call debug_hex_byte
    mov al,bl
    call debug_hex_byte
    pop dx
    pop bx
    pop ax
    ret
	
; ============================================================
; dword
;
; DX:AX
;
; ============================================================
debug_hex_dword:
    push ax
    push bx
    push dx
    mov bx,ax
    mov ax,dx
    call debug_hex_word
    mov ax,bx
    call debug_hex_word
    pop dx
    pop bx
    pop ax
    ret

; ============================================================
; single char
;
; AL
;
; ============================================================
debug_out_char:
    push dx
    mov dx,0xE9
    out dx,al
    pop dx
    ret	
	
; ------------------------------------------------------------
; newline
;
; вывод CR LF через debug port
;
; ------------------------------------------------------------
newline:
    push ax
    push dx
    mov dx,0xE9
    mov al,13
    out dx,al
    mov al,10
    out dx,al
    pop dx
    pop ax
    ret	
	
; ============================================================
; dump_inode
;
; ES:0000 -> inode buffer
;
; Выводит основные поля inode EXT2
;
; ============================================================

; ============================================================
; dump_inode
;
; DS = INODE_SEG
; SI = offset inode внутри блока
;
; ============================================================


; ============================================================
; dump_inode
;
; DS:SI -> inode
;
; EXT2 inode:
; +0  mode
; +2  uid
; +4  size low
; +40 block[0]
;
; ============================================================
dump_inode:
    push ax
    push si
    mov ax,[si+0]
    call debug_hex_word
    mov ax,[si+2]
    call debug_hex_word
    mov ax,[si+4]
    call debug_hex_word
    mov ax,[si+40]
    call debug_hex_word
    call newline
    pop si
    pop ax
    ret
; ============================================================
; dump_inode_bytes
;
; DS:SI -> inode
;
; dump first 64 bytes
;
; ============================================================
dump_inode_bytes:
    push ax
    push bx
    push cx
    push si
    xor bx,bx
    mov cx,32
	
.loop:
    mov ax,[si+bx]
    push bx
    push cx
    push si
    call debug_hex_word
    pop si
    pop cx
    pop bx
    add bx,2
    loop .loop
    call newline
    pop si
    pop cx
    pop bx
    pop ax
    ret
	
	
print_char:
    push ax
    push bx
    mov ah, 0x0E
    int 0x10
    pop bx
    pop ax
    ret
	
	
; ============================================================
; dump_kernel
;
; Dump первых 32 байт ядра
;
; ============================================================

dump_kernel:
    push ax
    push bx
    push cx
    push dx
    push si
    push ds
    mov ax,KERNEL_SEG
    mov ds,ax
    xor si,si
    mov cx,32
	
.next:
    lodsb
    ; ---------- старшая тетрада ----------
    mov ah,al
    shr ah,4
    mov al,ah
    call .print_nibble
    ; ---------- младшая ----------
    mov al,[si-1]
    and al,0Fh
    call .print_nibble
    mov al,' '
    mov ah,0Eh
    int 10h
    ; каждые 16 байт новая строка
    test si,15
    jnz .cont
    mov al,13
    mov ah,0Eh
    int 10h
    mov al,10
    mov ah,0Eh
    int 10h

.cont:
    loop .next
    pop ds
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; ============================================================
; AL = nibble (0..15)
; ============================================================
.print_nibble:
    cmp al,10
    jb .digit
    add al,'A'-10
    jmp .out
	
.digit:
    add al,'0'

.out:
    mov ah,0Eh
    int 10h
    ret

; ============================================================
; Messages
; ============================================================
	
msg_inode db "INODE",13,10,0
msg_mode db "MODE: ",0
msg_size db "SIZE: ",0
msg_block0 db "BLOCK0: ",0
msg_block1 db "BLOCK1: ",0
msg_kernel_dump db "KERNEL DUMP: ",0