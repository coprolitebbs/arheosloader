#!/bin/bash
set -e

GREEN='\033[0;32m'
NC='\033[0m'

echo -e "${GREEN}=== Building bootloader stages and kernel ===${NC}"

# ---- Компиляция загрузчиков ----
nasm -f bin boot_720.asm -o bin/boot_720.bin
nasm -f bin boot_1440.asm -o bin/boot_1440.bin
nasm -f bin boot_hdd.asm -o bin/boot_hdd.bin
nasm -f bin f12ld.asm -o bin/f12ld.bin
nasm -f bin hddld.asm -o bin/hddld.bin
#xxd -l 16 bin/f12ld.bin

# ---- Дополнение Stage2 до 8 КБ ----
STAGE2_SIZE=8192
echo "STAGE2 SIZE $STAGE2_SIZE"
dd if=bin/f12ld.bin of=bin/f12ld.pad bs=1 count=8192 conv=sync
xxd -l 16 bin/f12ld.pad
mv bin/f12ld.pad bin/f12ld.bin
#ls -l bin/f12ld.bin

# ---- Ядро ----
if command -v i686-elf-gcc &> /dev/null; then
    CC=i686-elf-gcc
    LD=i686-elf-ld
    OBJCOPY=i686-elf-objcopy
else
    echo "Error: i686-elf-gcc not found."
    exit 1
fi

$CC -m32 -ffreestanding -nostdlib -static -c kernel.c -o bin/kernel.o
if $LD -m elf_i386 -Ttext 0x100000 -o bin/kernel.bin bin/kernel.o --oformat binary 2>/dev/null; then
    echo "Linked with --oformat binary"
else
    echo "Falling back to ELF + objcopy"
    $LD -m elf_i386 -Ttext 0x100000 -o bin/kernel.elf bin/kernel.o
    $OBJCOPY -O binary bin/kernel.elf bin/kernel.bin
    rm -f bin/kernel.elf
fi
rm -f bin/kernel.o

# ---- Подготовка содержимого для дискет ----
mkdir -p output tmp_fat12
cp bin/f12ld.bin tmp_fat12/F12.LD
cp bin/kernel.bin tmp_fat12/KERNEL

# ---- FAT-образы (дискеты) ----
create_fat_image() {
    local IMG=$1
    local SIZE=$2
    local BOOT_BIN=$3

    echo -e "${GREEN}Creating FAT12 image: $IMG (${SIZE}K) with $BOOT_BIN${NC}"

    dd if=/dev/zero of="$IMG" bs=1024 count="$SIZE" 2>/dev/null
	if [ "$SIZE" -eq 1440 ]; then
        mkfs.fat -F 12 -f 2 -s 1 -r 224 -R 1 -v "$IMG" 2>/dev/null
    else
		mkfs.fat -F 12 -f 2 -s 2 -r 224 -R 1 -v "$IMG" 2>/dev/null
	fi
    # Копируем файлы в образ
    mcopy -i "$IMG" tmp_fat12/F12.LD ::/
    mcopy -i "$IMG" tmp_fat12/KERNEL ::/
	echo "xxd tmp_fat12/KERNEL   :"
	#xxd tmp_fat12/KERNEL

    # ---- ПРОСТО ПЕРЕЗАПИСЫВАЕМ ВЕСЬ ЗАГРУЗОЧНЫЙ СЕКТОР ----
    dd if="$BOOT_BIN" of="$IMG" conv=notrunc 2>/dev/null

    echo "Files in $IMG after boot sector write:"
    mdir -i "$IMG" ::/ || echo "mdir failed"
}

create_fat_image output/fat12_720.img 720 bin/boot_720.bin
create_fat_image output/fat12_1440.img 1440 bin/boot_1440.bin

rm -rf tmp_fat12

# ---- Подготовка содержимого для HDD (EXT2) ----
mkdir -p tmp_hdd
cp bin/hddld.bin tmp_hdd/EXT.LD
cp bin/kernel.bin tmp_hdd/kernel

# ---- Создание HDD образа (EXT2 + LBA) ----
echo -e "${GREEN}Creating HDD image (64M) with EXT2 and LBA...${NC}"
HDD_IMG="output/hdd_ext2.img"
dd if=/dev/zero of="$HDD_IMG" bs=1M count=64 2>/dev/null

if command -v genext2fs &> /dev/null; then
    genext2fs -b 65536 -d tmp_hdd "$HDD_IMG"
elif command -v mkfs.ext2 &> /dev/null; then
    echo "genext2fs not found, trying mkfs.ext2 with loop (may need sudo)..."
    LOOP=$(sudo losetup -f --show "$HDD_IMG")
    sudo mkfs.ext2 -r 0 -b 1024 "$LOOP" 2>/dev/null
    sudo mkdir -p /mnt/hdd_ext2
    sudo mount "$LOOP" /mnt/hdd_ext2
    sudo cp -r tmp_hdd/* /mnt/hdd_ext2/
    sudo umount /mnt/hdd_ext2
    sudo losetup -d "$LOOP"
else
    echo "Error: neither genext2fs nor mkfs.ext2 found. Install genext2fs or e2fsprogs."
    exit 1
fi

# ---- Запись загрузчиков в HDD ----
#dd if=bin/boot_hdd.bin of="$HDD_IMG" conv=notrunc 2>/dev/null
#dd if=bin/hddld.bin of="$HDD_IMG" bs=512 seek=1 conv=notrunc 2>/dev/null
dd if=bin/boot_hdd.bin of="$HDD_IMG" conv=notrunc 2>/dev/null
dd if=bin/hddld.bin of="$HDD_IMG" bs=512 seek=17 conv=notrunc 2>/dev/null

rm -rf tmp_hdd

echo -e "${GREEN}All images created successfully in output/${NC}"
echo -e "${GREEN}Images:${NC}"
ls -lh output/

# ---- Запуск QEMU с дискетой (по умолчанию) ----
#qemu-system-i386 -drive file=output/fat12_720.img,format=raw,if=floppy -vga std -display cocoa -debugcon stdio -global isa-debugcon.iobase=0xe9
qemu-system-i386 -drive file=output/hdd_ext2.img,format=raw,if=ide -vga std -display cocoa -debugcon stdio -global isa-debugcon.iobase=0xe9