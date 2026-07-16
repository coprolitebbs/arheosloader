# Archeosloader

**Archeosloader** — экспериментальный загрузчик операционной системы x86, написанный полностью с нуля на NASM Assembly.

Проект написан в академических целях для изучения низкоуровневой загрузки компьютера: работы BIOS, файловой системы EXT2, VBE-графики, защищённого режима процессора и передачи управления собственному ядру.

---

## Возможности

На текущем этапе Archeosloader умеет:

- Загружаться через BIOS (Legacy BIOS boot)
- Работать с HDD-образом через INT 13h Extensions (LBA)
- Читать файловую систему EXT2 или FAT12
- Находить и загружать kernel по inode
- Работать с прямыми блоками EXT2 inode
- Загружать ядро размером до ~1 МБ
- Проверять фактически загруженный размер ядра
- Включать линию A20
- Инициализировать VBE видеорежим
- Настраивать GDT
- Переключаться в Protected Mode (x86 32-bit)
- Передавать управление ядру

---

## Архитектура

Схема загрузки:


BIOS -> bootloader (stage1) -> stage2 (драйвер EXT2 или FAT12, Загрузка ядра, Инициализация VBE, Включение A20, установка GDT, Защищенный режим, Передача управления ядру)

---

## Этапы загрузки

### Stage 1

Первый загрузочный сектор:

- находится в MBR
- выполняется BIOS
- загружает второй этап загрузчика

Stage1 выполняет только минимальную работу:

- настройка окружения
- поиск stage2
- передача управления

bootloader представлен в трех вариантах: 

- boot_720.asm (output/fat12_720.img) - загрузчик для дискет 5.25 720Кб
- boot_1440.asm (output/fat12_1440.img) - загрузчик для дискет 3.5 1.44Мб
- boot_hdd.asm (output/hdd_ext2.img) - загрузчик для диска IDE с файловой системой EXT2


---

### Stage 2

Основная логика загрузчика:

- работа с диском
- чтение EXT2 или FAT12
- загрузка ядра
- настройка оборудования
- переход в protected mode
- передача управления ядру

stage2-loader представлен в двух вариантах:

- f12ld.asm - загрузчик ядра с файловой системы FAT12 дискеты
- hddld.asm - загрузчик ядра с файловой системы EXT2 жесткого диска

---

### Stage 3

Основное ядро:

На текущий момент заглушка в виде kernel.c - выводит сообщения на синем экране. 

---

## EXT2 Loader

Archeosloader содержит собственный минимальный EXT2 драйвер.

Поддерживается:

- Superblock
- Group Descriptor Table
- Inode table
- Directory entries
- Чтение inode ядра
- Direct blocks

На текущем этапе используются только прямые блоки EXT2.

В будущем планируется:

- Single Indirect blocks
- Double Indirect blocks
- загрузка больших ядер


---

## Kernel

Ядро собирается отдельно:


kernel.bin


На данный момент загрузчик ожидает бинарный образ ядра:


Размер ядра определяется напрямую из inode EXT2:


inode.i_size


После загрузки размер проверяется:


loaded_size == kernel_size


---

## Передача управления ядру

После загрузки:

1. Ядро находится в памяти
2. Включается Protected Mode
3. Загружаются сегменты GDT
4. Управление передается:


jmp 0x08:0x100000


Перед запуском ядру передаются параметры:


EDI = framebuffer address

ESI = framebuffer pitch

EDX = bytes per pixel


---

## Видеорежим

Используется:

- BIOS VBE
- Linear Framebuffer

Получаются параметры:


LFB address
Pitch
Bits per pixel
Resolution


Эти параметры передаются ядру.

---

## Сборка

Сборка происходит автоматически с помощью скрипта build.sh

Требуется:

- nasm
- qemu
- dosfstools
- genext2fs
- mtools
- e2fsprogs

Сборка проверялась на Mac Os, все пакеты можно установить через brew install


Пример:

```bash
nasm boot.asm -o boot.bin

nasm stage2.asm -o stage2.bin

cat boot.bin stage2.bin > disk.img
```

Запуск

Пример запуска через QEMU:

- Запуск с образа дискеты:

```
qemu-system-i386 -drive file=output/fat12_720.img,format=raw,if=floppy -vga std -display cocoa -debugcon stdio -global isa-debugcon.iobase=0xe9
```

- Запуск с образа hdd:

```
qemu-system-i386 -drive file=output/hdd_ext2.img,format=raw,if=ide -vga std -display cocoa -debugcon stdio -global isa-debugcon.iobase=0xe9
```

# arheosloader
