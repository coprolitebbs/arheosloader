BITS 16
ORG 0x7E00


; ============================================================
; Stage2 EXT2 loader test
;
; Сейчас:
;
; - BIOS LBA
; - EXT2 superblock
; - GDT
; - inode table
; - inode read
;
; ============================================================

    cli

; ------------------------------------------------------------
; setup segments
; ------------------------------------------------------------
    xor ax,ax
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,0x9C00
    sti
	
	push es
; ------------------------------------------------------------
; save BIOS drive
; ------------------------------------------------------------
	mov [drive],dl
; ------------------------------------------------------------
; init disk
; ------------------------------------------------------------
    call ext2_disk_init
; ------------------------------------------------------------
; read EXT2 superblock
; ------------------------------------------------------------
    call ext2_init
    jc halt
	
	;mov ax,5
	;call ext2_read_block_test
    ;mov si,msg_after_super
    ;call print_debug
	
    call ext2_read_group_table
    jc halt
; ------------------------------------------------------------
; DEBUG GDT
; ------------------------------------------------------------
	;push si
	;push ds
	;mov ax,GDT_SEG
	;mov ds,ax
	;xor si,si
	;mov ax,[si+0]
	;call debug_hex_word
	;mov ax,[si+2]
	;call debug_hex_word
	;mov ax,[si+4]
	;call debug_hex_word
	;mov ax,[si+6]
	;call debug_hex_word
	;mov ax,[si+8]
	;call debug_hex_word
	;call newline
	;pop ds
	;pop si
    ;mov si,msg_after_gdt
    ;call print_debug
; ------------------------------------------------------------
; group 0
;
; ------------------------------------------------------------
	
    ;mov ax,6
	;call ext2_get_group_inode_table
	;mov si,msg_ds_test
	;call print_debug
	;mov ax,ds
	;call debug_hex_word
	;call newline
; ----------------------------------------
; DEBUG inode table address
; ----------------------------------------
	;push ds
	;xor ax,ax
	;mov ds,ax
	;mov si,msg_test
	;call print_debug
	;mov ax,[cs:ext2_inode_table+2]
	;call debug_hex_word
	;mov ax,[cs:ext2_inode_table]
	;call debug_hex_word
	;call newline
	;pop ds
; ----------------------------------------
; TEST READ INODE TABLE FIRST BLOCK
;
; ext2_inode_table = block number
;
; ----------------------------------------
	;mov ax,[cs:ext2_inode_table]
	;call ext2_read_block_test
	;jc halt
; ----------------------------------------
; dump first bytes of block
; ----------------------------------------
	;push ds
	;mov ax,INODE_SEG
	;mov ds,ax
	;call dump_inode_bytes
	;pop ds
; ------------------------------------------------------------
; read inode 97
; ------------------------------------------------------------
	;mov ax,97
	;push ax
	;call debug_hex_word
	;call newline
	;pop ax
	;call ext2_read_inode
	;jc halt
	;mov si,msg_inode_ok
	;call print_debug

	mov ax,0
	call ext2_get_group_inode_table
	
	;mov ax,[cs:ext2_inode_table]
	;call debug_hex_word
	;call newline

	mov ax,2
	call ext2_read_inode
	jc halt
	
	;mov si,msg_inode2_ok
	;call print_debug

; root directory

	call ext2_read_root_dir
	jc halt
	
	;push ds
	;xor ax,ax
	;mov ds,ax
	;mov si,msg_before_kernel_load
	;call print_debug
	;mov ax,[ext2_inode_table]
	;call debug_hex_word
	;mov ax,[ext2_inode_table+2]
	;call debug_hex_word
	;call newline
	;pop ds

	mov ax,6
	call ext2_get_group_inode_table

	call ext2_load_kernel
	jc halt
	
	;mov ax,KERNEL_SEG
	;mov ds,ax
	;xor si,si
	;mov al,[ds:si]
	;call debug_hex_byte
	;mov al,[ds:si+1]
	;call debug_hex_byte
	;mov al,[ds:si+2]
	;call debug_hex_byte
	;mov al,[ds:si+3]
	;call debug_hex_byte
	;call newline
	
	mov eax,[kernel_size]
	mov [kernel_size_pm],eax
	
	call dump_kernel
	
	pop es

	cli

	; ---- Инициализация VBE ----
    call vbe_init

    ; ---- Инициализация A20 ----
    call a20_init

    ; ---- Переход в защищённый режим ----
    lgdt [gdtr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_entry
BITS 32
pm_entry:
    ; ---- Отладка через видеопамять ----
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    cli	
	
	; ---- Копирование ядра в 100000 ----
	push esi
    push edi
    push ecx
	cld
	mov esi,0x20000
	mov edi,0x100000
	mov ecx,[kernel_size_pm]
	rep movsb
	pop ecx
    pop edi
    pop esi
	
	cli
	
	; ---- Запуск ядра ----
	mov edi, [lfb_addr]
    mov esi, [pitch]
    mov edx, [bytes_pp]
    jmp 0x08:0x100000

    jmp halt32

halt32:
    cli

.loop32:
    hlt
    jmp .loop32
	
copy_error:
    mov si, msg_copy_error
    call print_debug
    jmp halt



BITS 16

halt:
    cli

.loop:
    hlt
    jmp .loop

; ============================================================
; Variables
; ============================================================


BITS 16

kernel_size_pm dd 0
drive: db 0

; ============================================================
; Messages
; ============================================================

;msg_after_super: db "AFTER SUPER",13,10,0
;msg_test: db "msg_test (old MSG_INODE_TABLE): ",0
;msg_inode_ok db "INODE 97 OK",13,10,0
;msg_inode2_ok db "INODE 2 OK",13,10,0
;msg_inode1_ok db "INODE 1 OK",13,10,0	
;msg_ds_test    db "DS=",0
;msg_before_kernel_load db "BEFORE KERNEL LOAD TABLE: ",0
msg_copy_error db "KERNEL COPY ERR ",0

; ============================================================
; Includes
; ============================================================

%include "include/ext2disk.inc"
%include "include/ext2debug.inc"
%include "include/ext2_stage2.inc"
%include "include/ext2_group.inc"
%include "include/ext2_inode.inc"
%include "include/ext2_dir.inc"
%include "include/ext2_load.inc"
%include "include/vbe.inc"
%include "include/gdt.inc"
%include "include/graphics.inc"
%include "include/font.inc"


BITS 16
; ============================================================
; Stage2 padding
; ============================================================

times 8192-($-$$) db 0