BITS 16

KERNEL_SEG equ 0x2000

; ============================================================
; ext2_load_kernel
;
; Загружает kernel по inode из kernel_inode
;
; Требует:
;
; kernel_inode
; ext2_read_inode
; ext2_read_sectors
;
; Загружает в:
;
; KERNEL_SEG:0000
;
; ============================================================

ext2_load_kernel:	
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push ds
    push es
	mov word [cs:kernel_load_offset],0
	mov word [cs:kernel_load_offset+2],0	
; ------------------------------------------------
; restore inode table
; ------------------------------------------------
    mov ax,6
    call ext2_get_group_inode_table
; ------------------------------------------------
; read kernel inode
; ------------------------------------------------
    mov ax,[cs:kernel_inode]
    call ext2_read_inode
    jc .error
; ------------------------------------------------
; inode buffer
; ------------------------------------------------
    mov ax,INODE_SEG
    mov ds,ax
    mov si,[cs:inode_offset]	
	mov [cs:kernel_inode_offset],si	
; ------------------------------------------------
; read kernel size from inode
;
; i_size offset = +4
;
; ------------------------------------------------
    mov ax,[si+4]
    mov [cs:kernel_size],ax
    mov ax,[si+6]
    mov [cs:kernel_size+2],ax
	
	push ds
	xor ax,ax
	mov ds,ax
	mov si,msg_kernel_size
	call print_debug
	mov ax,[cs:kernel_size+2]
	call debug_hex_word
	mov ax,[cs:kernel_size]
	call debug_hex_word
	call newline
	pop ds

; ------------------------------------------------
; direct blocks
; ------------------------------------------------
	mov word [cs:kernel_block_index],0

.next_block:
; ------------------------------------------------
; restore block index
; ------------------------------------------------
    mov di,[cs:kernel_block_index]
; ------------------------------------------------
; check loaded size
; ------------------------------------------------
    mov ax,[cs:kernel_load_offset]
    mov dx,[cs:kernel_load_offset+2]
    cmp dx,[cs:kernel_size+2]
    jb .need_block
    ja .done
    cmp dx,[cs:kernel_size+2]
	jb .ok_size
	ja .size_error
	cmp ax,[cs:kernel_size]
	jb .ok_size

.ok_size:

.need_block:
	mov si,[cs:kernel_inode_offset]	
    mov bx,si
    add bx,40
    add bx,di
    mov ax,[bx]
    test ax,ax
    jz .done
	
; debug
    push ax
    push bx
    push ds
    xor ax,ax
    mov ds,ax
    mov si,msg_kernel_block
    call print_debug
    pop ds
    pop bx
    pop ax
    call debug_hex_word
    call newline
; ------------------------------------------------
; block -> LBA
; ------------------------------------------------
    xor dx,dx
    shl ax,1
    rcl dx,1
	push ax
    push dx
; ------------------------------------------------
; destination buffer
; ------------------------------------------------
	mov ax,KERNEL_SEG
	mov bx,[cs:kernel_load_offset]
	mov dx,bx
	shr dx,4
	add ax,dx
	mov es,ax
	and bx,0x000F
	mov cx,2
	pop dx
    pop ax	
	
	push ax
	push dx
	push ds
	xor ax,ax
	mov ds,ax
	mov si,msg_kernel_lba
	call print_debug
	pop ds
	pop dx
	pop ax


	push ax
	push dx
	call debug_hex_dword
	call newline
	pop dx
	pop ax

	call ext2_read_sectors
    jc .error

	add word [cs:kernel_block_index],4
	add word [cs:kernel_load_offset],1024
	adc word [cs:kernel_load_offset+2],0
; если загрузили достаточно
	mov ax,[cs:kernel_load_offset]
	mov dx,[cs:kernel_load_offset+2]
	cmp dx,[cs:kernel_size+2]
	jb .continue
	ja .done
	cmp ax,[cs:kernel_size]
	jae .done

.continue:
	jmp .next_block

.done:
    mov ax,[cs:kernel_load_offset]
    mov dx,[cs:kernel_load_offset+2]
    cmp dx,[cs:kernel_size+2]
    jb .size_error
    ja .ok
    cmp ax,[cs:kernel_size]
    jb .size_error
	
.ok:
    push ds
    xor ax,ax
    mov ds,ax
    mov si,msg_kernel_loaded
    call print_debug
    pop ds

    clc
    jmp .exit

.error:
    push ds
    xor ax,ax
    mov ds,ax
    mov si,msg_kernel_error
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
	
.size_error:
    mov si,msg_kernel_size_error
    call print_debug
    stc
    jmp .exit
	
; ============================================================
; variables
; ============================================================
	
kernel_load_offset dd 0
kernel_size dd 0
kernel_inode_offset dw 0
kernel_block_index dw 0

; ============================================================
; messages
; ============================================================
msg_kernel_block db "KERNEL BLOCK: ",0
msg_kernel_loaded db "KERNEL LOADED",13,10,0
msg_kernel_error db "KERNEL LOAD ERROR",13,10,0	
msg_kernel_lba db "KERNEL LBA: ",0	
msg_kernel_size_error db "KERNEL SIZE ERROR",13,10,0	
msg_kernel_size db "KERNEL SIZE: ",13,10,0