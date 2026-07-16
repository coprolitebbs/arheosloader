; ============================================================
; ext2_dir.inc
;
; EXT2 root directory reader
;
; Требует:
;
; ext2_read_inode
; ext2_read_sectors
; ext2_block_size
;
; После выполнения:
;
; kernel_inode содержит inode файла kernel
;
; ============================================================

BITS 16
DIR_SEG equ 0x1600
; ============================================================
; ext2_read_root_dir
;
; Читает inode 2 (root)
;
; ищет "kernel"
;
; ============================================================

ext2_read_root_dir:
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push ds	
	xor ax,ax
	mov ds,ax	
    push es
    ; --------------------------------
    ; read inode 2
    ; --------------------------------
    mov ax,2
    call ext2_read_inode
    jc .error
    ; --------------------------------
    ; inode buffer
    ; --------------------------------
    mov ax,INODE_SEG
    mov ds,ax
    mov si,[cs:inode_offset]
    ; --------------------------------
    ; i_block[0]
    ; --------------------------------
    mov ax,[si+40]
    
	;push ax
    ;xor ax,ax
    ;mov ds,ax
    ;mov si,msg_root_block
    ;call print_debug
    ;pop ax
    ;call debug_hex_word
    ;call newline
    
	; восстановить DS inode
    mov ax,INODE_SEG
    mov ds,ax
    mov si,[cs:inode_offset]
    mov ax,[si+40]
    ; --------------------------------
    ; block -> LBA
    ; --------------------------------
    cmp word [cs:ext2_block_size],1024
    jne .error
    shl ax,1
    xor dx,dx
    ; сохранить LBA
    push ax
    push dx
    ; buffer
    mov ax,DIR_SEG
    mov es,ax
    xor bx,bx
    mov cx,2
    pop dx
    pop ax	
	
	;push ax
    ;push dx
    ;mov si,msg_dir_lba
    ;xor ax,ax
    ;mov ds,ax
    ;call print_debug
    ;pop dx
    ;pop ax

    ;push ax
    ;push dx
    ;call debug_hex_dword
    ;call newline
    ;pop dx
    ;pop ax
	
    call ext2_read_sectors
    jc .error
    ; --------------------------------
    ; parse directory
    ; --------------------------------
    mov ax,DIR_SEG
    mov ds,ax
    xor si,si

.next:
    ; inode number
    mov ax,[si]
    mov dx,[si+2]
    or ax,dx
    jz .end
    ; save inode
    push ax
    push dx
    ; rec_len
    mov bx,[si+4]
    cmp bx,0
    je .end_bad
    ; name length
    xor cx,cx
    mov cl,[si+6]
    cmp cx,6
	; Очень тупо сравниваем с "kernel", но, это не Олимпийские игры, плевать 
    jne .skip_restore
    cmp byte [si+8],'k'
    jne .skip_restore
    cmp byte [si+9],'e'
    jne .skip_restore
    cmp byte [si+10],'r'
    jne .skip_restore
    cmp byte [si+11],'n'
    jne .skip_restore
    cmp byte [si+12],'e'
    jne .skip_restore
    cmp byte [si+13],'l'
    jne .skip_restore
    ; found
    pop dx
    pop ax
    mov [cs:kernel_inode],ax
    
	;push ds
    ;xor ax,ax
    ;mov ds,ax
    ;mov si,msg_kernel_found
    ;call print_debug
    ;mov ax,[cs:kernel_inode]
    ;call debug_hex_word
    ;call newline	
    ;pop ds
	
    clc
    jmp .exit

.skip_restore:
    pop dx
    pop ax
    add si,bx
    jmp .next

.end_bad:
    pop dx
    pop ax

.end:
    push ds
    xor ax,ax
    mov ds,ax
    mov si,msg_kernel_not_found
    call print_debug
    pop ds
    stc
    jmp .exit

.error:
    push ds
    xor ax,ax
    mov ds,ax
    mov si,msg_dir_error
    call print_debug
    pop ds
    stc

.exit:
    pop es
    pop ds
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; ============================================================
; variables
; ============================================================

kernel_inode dw 0

; ============================================================
; messages
; ============================================================
;msg_kernel_found db "KERNEL INODE: ",0
;msg_dir_lba db "DIR LBA: ",0
msg_kernel_not_found db "KERNEL NOT FOUND",13,10,0
msg_dir_error db "DIR ERROR",13,10,0	
;msg_root_block db "ROOT BLOCK: ",0