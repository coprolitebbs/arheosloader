; ---------- Функции чтения ----------
disk_read_sectors:
    pusha
.loop:
    push ax
    push cx
    call disk_read_sector
    pop cx
    pop ax
    jc .error
    add bx, 512
    inc ax
    loop .loop
    popa
    clc
    ret
.error:
    popa
    stc
    ret

disk_read_sector:
    push ax
    push bx
    push cx
    push dx

    xor dx, dx
    div word [spt]
    inc dx
    mov cl, dl
    xor dx, dx
    div word [heads]
    mov ch, al
    mov dh, dl
    mov al, ah
    and al, 3
    shl al, 6
    or cl, al

    mov dl, [drive]
    mov ah, 0x02
    mov al, 1
    int 0x13

    pop dx
    pop cx
    pop bx
    pop ax
    ret