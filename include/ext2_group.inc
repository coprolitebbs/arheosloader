; ============================================================
; ext2_group.inc
;
; EXT2 group descriptor table
;
; Requires:
;
; ext2_read_sectors
; ext2_block_size
; ext2_first_data_block
;
; ============================================================
BITS 16
GDT_SEG equ 0x1200
; ============================================================
; ext2_read_group_table
;
; Читает первый блок GDT
;
; ============================================================

ext2_read_group_table:
    push ax
    push bx
    push cx
    push dx
    mov ax,GDT_SEG
    mov es,ax
    xor bx,bx

    ; GDT начинается:
    ;
    ; first_data_block + 1
    ;
    ; для block size 1024:
    ;
    ; block 2
    ; LBA 4

    mov ax,[ext2_first_data_block]
    inc ax
	
    ; block -> LBA
    ;
    ; 1024 bytes = 2 sectors
	
    shl ax,1
    xor dx,dx
    mov cx,2
    call ext2_read_sectors
    jc .error
    pop dx
    pop cx
    pop bx
    pop ax
    clc
    ret
	
.error:
    mov si,msg_group_error
    call print_debug

    pop dx
    pop cx
    pop bx
    pop ax
    stc
    ret
	
; ============================================================
; ext2_get_group_inode_table
;
; IN:
;
; AX = group number
;
; OUT:
;
; ext2_inode_table = inode table block
;
; ============================================================
ext2_get_group_inode_table:
    push ax
    push bx
    push dx
    push si
    push ds
    xor dx,dx
    mov bx,32
    mul bx
    mov si,ax
    mov ax,GDT_SEG
    mov ds,ax
    mov ax,[si+8]
    mov dx,[si+10]
    mov [cs:ext2_inode_table],ax
    mov [cs:ext2_inode_table+2],dx
    pop ds
    pop si
    pop dx
    pop bx
    pop ax
    ret

; ============================================================
; Debug dump
;
; ============================================================
;dump_group_descriptor:
    ;push ax
    ;push dx
    ;mov si,msg_group_inode_table
    ;call print_debug
    ;mov ax,[cs:ext2_inode_table]
    ;call debug_hex_word
    ;mov ax,[cs:ext2_inode_table+2]
    ;call debug_hex_word
    ;call newline
    ;pop dx
    ;pop ax
    ;ret

; ============================================================
; Variables
; ============================================================

; ============================================================
; Messages
; ============================================================

;msg_after_gdt: db "AFTER GDT",13,10,0
msg_group_error db "GROUP ERROR",13,10,0
;msg_group_inode_table db "INODE TABLE: ",0