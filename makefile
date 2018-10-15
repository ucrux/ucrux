# bootload入口地址
BOOTLOAD_ENTRYPOINT = 0x00091000

# 内核入口地址
# 必须和'const.inc.asm'中的KERNEL_ENTRY_ADDR一样
KERNEL_ENTRYPOINT	= 0x00011000
# 用户进程或者服务进程的入口地址
PROC_ENTRYPOINT = 0x40002000
# 启动磁盘路径
HD		= build/hda.vhd

# Programs, flags, etc.
ASM		= nasm
CC		= gcc
LD		= ld
C_INCLUDE_DIR = -I include/ -I kernel/include/ \
                -I lib/kernel/include/ -I lib/common/include/ \
                -I boot/include/ -I kernel/mm/include/ \
                -I kernel/int_handle/include/ \
                -I kernel/int_handle/device/include/ \
                -I syscall/include/ \
                -I kernel/proc/include/ \
                -I kernel/msg/include/ \
                -I kernel/hdisk/include/ \
                -I syscall/include/ \
                -I task/video/include/ \
                -I task/tty/include/ \
                -I task/keyboard/include/
ASM_INCLUDE_DIR	= -I boot/include/ -I lib/kernel/include/ \
                  -I kernel/int_handle/include/ \
                  -I kernel/hdisk/include/
CFLAGS		= -m32 -Wall $(C_INCLUDE_DIR) -c -fno-builtin -W \
            -Wstrict-prototypes \
            -Wmissing-prototypes \
            -fno-stack-protector
BOOTLOAD_LDFLAGS = -m elf_i386 -Ttext $(BOOTLOAD_ENTRYPOINT) -e main
KERNEL_LDFLAGS	 = -m elf_i386 -Ttext $(KERNEL_ENTRYPOINT) -e main
PROC_LDFLAGS = -m elf_i386 -Ttext $(PROC_ENTRYPOINT) -e main



# 源码文件
MBR_SRC_FILE = boot/mbr.asm
LIB_BIN_TO_BCD_SRC_FILE = lib/common/binary32_to_packed_bcd.c
DISP_CHAR_SRC_FILE = lib/kernel/disp_char_with_color.asm
ELF_LOAD_SRC_FILE = lib/kernel/elf_load.asm
KPUTX_SRC_FILE = lib/kernel/kputx.c
BOOTLOAD_SRC_FILE = boot/bootload.c
READKERNEL_SRC_FILE = boot/readkernel.asm
DLIST_SRC_FILE = lib/common/dlist.c
BUDDY_SRC_FILE = kernel/mm/buddy_sys.c
STRING_SRC_FILE = lib/common/string.c
INTR_HANDLE_ASM_SRC_FILE = kernel/int_handle/int_handle.asm
IDT_INIT_SRC_FILE = kernel/int_handle/idt_init.c
PIC_SRC_FILE = kernel/int_handle/device/pic.c
TIMER_8253_SRC_FILE = kernel/int_handle/device/timer_8253.c
TIMER_INTR_HANDLE_SRC_FILE = kernel/int_handle/timer_intr_handle.c
INTERRUPT_SRC_FILE = kernel/int_handle/interrupt.c
DEBUG_SRC_FILE = lib/common/debug.c
BIN_TREE_SRC_FILE = lib/common/binary_tree.c
AVL_TREE_SRC_FILE = lib/common/avl_tree.c
BITMAP_SRC_FILE = lib/common/bitmap.c
HASH_SRC_FILE = lib/common/hash.c
MM_SRC_FILE = kernel/mm/mm.c
KERNEL_MM_SRC_FILE = kernel/mm/kernel_mm.c
PROC_SRC_FILE = kernel/proc/proc.c
SWITCH_SRC_FILE = kernel/proc/switch_to.asm
KSYNC_SRC_FILE = lib/kernel/ksync.c
MSG_SRC_FILE = kernel/msg/msg.c
SYSHD_OPT_SRC_FILE = kernel/hdisk/syshd_opt.asm
KEYBOARD_INTR_SRC_FILE = kernel/int_handle/device/keyboard_intr.c
STDIO_SRC_FILE = lib/common/stdio.c
KSTDIO_SRC_FILE = lib/kernel/kstdio.c

PAGE_SRC_FILE = kernel/mm/page.c
KERNEL_SRC_FILE = kernel/kernel.c


# 服务进程
# 系统调用源文件
SYSCALL_SRC_FILE = syscall/syscall.c
# 服务进程
VIDEO_SRC_FILE = task/video/main.c
TTY_SRC_FILE = task/tty/main.c
KEYBOARD_SRC_FILE = task/keyboard/main.c




# 头文件
MBR_DEP_FILE = boot/include/const.inc.asm \
               boot/include/paging.inc.asm \
               boot/include/protect_mode.inc.asm \
               lib/kernel/include/elf_load.inc.asm
LIB_BIN_TO_BCD_DEP_FILE = lib/common/include/binary32_to_packed_bcd.h \
                          include/global_type.h
DISP_CHAR_DEP_FILE = lib/kernel/include/disp_char_with_color.inc.asm
ELF_LOAD_DEP_FILE = lib/kernel/include/elf_load.inc.asm
KPUTX_DEP_FILE = lib/kernel/include/kputx.h \
                 lib/kernel/include/disp_char_with_color.h \
                 ${LIB_BIN_TO_BCD_SRC_FILE} \
                 ${LIB_BIN_TO_BCD_DEP_FILE} \
                 ${DISP_CHAR_SRC_FILE}
READKERNEL_DEP_FILE = boot/include/readkernel_const.inc.asm
BOOTLOAD_DEP_FILE = lib/kernel/include/elf_load.h \
                    boot/include/bootload.h \
                    ${READKERNEL_SRC_FILE} \
                    ${READKERNEL_DEP_FILE} \
                    ${ELF_LOAD_DEP_FILE} \
                    ${KPUTX_DEP_FILE}
DLIST_DEP_FILE = lib/common/include/dlist.h \
                 include/global_type.h
BUDDY_DEP_FILE = kernel/mm/include/buddy_sys.h \
                 ${DLIST_SRC_FILE} \
                 ${DLIST_DEP_FILE} \
                 include/global_type.h \
                 lib/common/include/get_struct_pointor.h
PAGE_DEP_FILE = kernel/mm/include/page.h \
                include/global_type.h \
                lib/common/include/avl_tree.h \
                kernel/include/kernel.h \
                kernel/mm/include/buddy_sys.h \
                lib/common/include/string.h \
                lib/common/include/get_struct_pointor.h
STRING_DEP_FILE = include/global_type.h \
                  lib/common/include/string.h
INTR_HANDLE_ASM_DEP_FILE = kernel/int_handle/include/int_handle.inc.asm
IDT_INIT_DEP_FILE = kernel/int_handle/include/idt_init.h \
                    include/global_type.h \
                    kernel/include/kernel.h
PIC_DEP_FILE = kernel/int_handle/device/include/pic.h \
               lib/kernel/include/io.h
TIMER_8253_DEF_FILE = kernel/int_handle/device/include/timer_8253.h \
											lib/kernel/include/io.h \
											include/global_type.h
TIMER_INTR_HANDLE_DEP_FILE = kernel/int_handle/include/timer_intr_handle.h \
                             kernel/proc/include/proc.h \
                             kernel/msg/include/msg.h \
                             lib/common/include/dlist.h \
                             lib/common/include/get_struct_pointor.h \
                             kernel/include/kernel_service.h

INTERRUPT_DEP_FILE = kernel/int_handle/include/interrupt.h \
                     kernel/int_handle/include/idt_init.h \
                     ${PIC_DEP_FILE} \
                     ${TIMER_8253_DEF_FILE} \
                     lib/kernel/include/kputx.h \
                     include/global_type.h \
                     kernel/proc/include/proc.h \
                     kernel/msg/include/msg.h \
                     kernel/mm/include/kernel_mm.h \
                     kernel/int_handle/device/include/keyboard_intr.h
DEBUG_DEP_FILE = lib/common/include/debug.h \
                 lib/kernel/include/kputx.h \
                 kernel/int_handle/include/interrupt.h
BIN_TREE_DEP_FILE = lib/common/include/binary_tree.h \
                    include/global_type.h
AVL_TREE_DEP_FILE = lib/common/include/binary_tree.h \
										lib/common/include/avl_tree.h \
										include/global_type.h
BITMAP_DEP_FILE = lib/common/include/bitmap.h \
                  include/global_type.h \
                  lib/common/include/string.h
HASH_DEP_FILE = include/global_type.h \
                lib/common/include/dlist.h
MM_DEP_FILE = kernel/mm/include/mm.h \
              lib/common/include/dlist.h \
              include/global_type.h \
              lib/common/include/bitmap.h \
              lib/common/include/hash.h \
              kernel/mm/include/page.h \
              lib/common/include/get_struct_pointor.h
KERNEL_MM_DEP_FILE = kernel/mm/include/kernel_mm.h \
										 lib/common/include/hash.h \
										 lib/common/include/dlist.h \
										 kernel/mm/include/buddy_sys.h \
										 kernel/include/kernel.h \
										 lib/common/include/get_struct_pointor.h \
										 include/global_type.h \
										 kernel/mm/include/page.h \
										 kernel/proc/include/proc.h
PROC_DEP_FILE = kernel/proc/include/proc.h \
                include/global_type.h \
                lib/common/include/dlist.h \
                kernel/mm/include/mm.h \
                kernel/mm/include/page.h \
                lib/common/include/string.h \
                kernel/include/kernel.h \
                lib/common/include/hash.h \
                kernel/mm/include/kernel_mm.h \
                lib/common/include/get_struct_pointor.h \
                lib/common/include/debug.h \
                kernel/msg/include/msg.h
KSYNC_DEP_FILE = lib/kernel/include/ksync.h \
                 include/global_type.h \
								 lib/common/include/dlist.h \
								 kernel/proc/include/proc.h \
								 lib/common/include/get_struct_pointor.h \
								 lib/common/include/debug.h
MSG_DEP_FILE = kernel/msg/include/msg.h \
               include/global_type.h \
               lib/common/include/dlist.h \
               kernel/proc/include/proc.h \
               lib/common/include/hash.h \
               lib/common/include/get_struct_pointor.h \
               lib/common/include/string.h \
               kernel/include/kernel.h \
               kernel/mm/include/kernel_mm.h \
               lib/kernel/include/ksync.h \
               lib/common/include/debug.h \
               kernel/include/kernel_service.h
SYSHD_OPT_DEP_FILE = kernel/hdisk/include/syshd_opt.inc.asm
KEYBOARD_INTR_DEP_FILE = kernel/int_handle/device/include/keyboard_intr.h \
                         task/keyboard/include/keyboard.h \
                         lib/kernel/include/io.h \
                         kernel/proc/include/proc.h \
                         kernel/msg/include/msg.h \
                         kernel/include/kernel_service.h \
                         include/global_type.h \
                         kernel/int_handle/include/interrupt.h

STDIO_DEP_FILE = lib/common/include/stdio.h \
                 lib/common/include/string.h
                 
KSTDIO_DEP_FILE = lib/kernel/include/kstdio.h \
                  lib/kernel/include/kputx.h \
                  lib/common/include/stdio.h


SYSCALL_DEP_FILE = syscall/include/syscall.h \
                   kernel/proc/include/proc.h \
                   kernel/int_handle/include/interrupt.h \
                   lib/common/include/string.h
VIDEO_DEP_FILE = task/video/include/video.h \
                 syscall/include/syscall.h \
                 lib/kernel/include/kputx.h \
                 include/global_type.h \
                 kernel/msg/include/msg.h
TTY_DEP_FILE = syscall/include/syscall.h \
               task/tty/include/tty.h \
               include/global_type.h \
               kernel/msg/include/msg.h \
               task/video/include/video.h \
               lib/common/include/string.h \
               kernel/proc/include/proc.h
KEYBOARD_DEP_FILE = task/keyboard/include/keyboard.h \
                    syscall/include/syscall.h \
                    kernel/proc/include/proc.h \
                    task/video/include/video.h \
                    kernel/msg/include/msg.h \
                    task/tty/include/tty.h \
                    include/global_type.h \
                    lib/common/include/string.h


KERNEL_DEP_FILE = kernel/include/kernel.h \
                  include/global_type.h \
                  kernel/include/kernel_service.h \
                  lib/kernel/include/elf_load.h \
                  lib/common/include/dlist.h \
                  ${KPUTX_DEP_FILE} \
                  ${KPUTX_DEP_FILE} \
                  kernel/int_handle/device/include/timer_8253.h



# obj文件

LIB_BIN_TO_BCD_OBJ = build/binary32_to_packed_bcd.o
DISP_CHAR_OBJ = build/disp_char_with_color.o
ELF_LOAD_OBJ = build/elf_load.o
KPUTX_OBJ = build/kputx.o
BOOTLOAD_OBJ = build/bootload.o
READKERNEL_OBJ = build/readkernel.o
DLIST_OBJ = build/dlist.o
BUDDY_OBJ = build/buddy.o
PAGE_OBJ = build/page.o
STRING_OBJ = build/string.o
INTR_HANDLE_ASM_OBJ = build/intr_handle.o
IDT_INIT_OBJ = build/idt_init.o
PIC_OBJ = build/pic.o
TIMER_8253_OBJ = build/timer_8253.o
TIMER_INTR_HANDLE_OBJ = build/timer_intr_handle.o
INTERRUPT_OBJ = build/interrupt.o
DEBUG_OBJ = build/debug.o
BIN_TREE_OBJ = build/binary_tree.o
AVL_TREE_OBJ = build/avl_tree.o
BITMAP_OBJ = build/bitmap.o
HASH_OBJ = build/hash.o
MM_OBJ = build/mm.o
KERNEL_MM_OBJ = build/kernel_mm.o
PROC_OBJ = build/proc.o
SWITCH_OBJ = build/switch_to.o
KSYNC_OBJ = build/ksync.o
MSG_OBJ = build/msg.o
SYSHD_OPT_OBJ = build/syshd_opt.o
KEYBOARD_INTR_OBJ = build/keyboard_intr.o
STDIO_OBJ = build/stdio.o
KSTDIO_OBJ = build/kstdio.o

SYSCALL_OBJ = build/syscall.o
VIDEO_OBJ = build/video.o
TTY_OBJ = build/tty.o
KEYBOARD_OBJ = build/keyboard.o

VIDEO_DEP_OBJ = ${SYSCALL_OBJ} \
								${STRING_OBJ} \
								${KPUTX_OBJ} \
								${DISP_CHAR_OBJ} \
								${LIB_BIN_TO_BCD_OBJ}


TTY_DEP_OBJ = ${SYSCALL_OBJ} \
              ${STRING_OBJ}

KEYBOARD_DEP_OBJ = ${SYSCALL_OBJ} \
                   ${STRING_OBJ} \
                


BOOTLOAD_DEP_OBJ = ${BOOTLOAD_OBJ} \
                   ${ELF_LOAD_OBJ} \
                   ${KPUTX_OBJ} \
                   ${LIB_BIN_TO_BCD_OBJ} \
                   ${DISP_CHAR_OBJ} \
                   ${READKERNEL_OBJ}
KERNEL_OBJ = build/kernel.o

KERNEL_DEP_OBJ = ${KERNEL_OBJ} \
                 ${KPUTX_OBJ} \
                 ${LIB_BIN_TO_BCD_OBJ} \
                 ${DISP_CHAR_OBJ} \
                 ${BUDDY_OBJ} \
                 ${DLIST_OBJ} \
                 ${PAGE_OBJ} \
                 ${STRING_OBJ} \
                 ${INTR_HANDLE_ASM_OBJ} \
                 ${IDT_INIT_OBJ} \
                 ${PIC_OBJ} \
                 ${TIMER_8253_OBJ} \
                 ${TIMER_INTR_HANDLE_OBJ} \
                 ${INTERRUPT_OBJ} \
                 ${DEBUG_OBJ} \
                 ${BIN_TREE_OBJ} \
                 ${AVL_TREE_OBJ} \
                 ${BITMAP_OBJ} \
                 ${HASH_OBJ} \
                 ${MM_OBJ} \
                 ${PROC_OBJ} \
                 $(KERNEL_MM_OBJ) \
                 ${KSYNC_OBJ} \
                 ${MSG_OBJ} \
                 ${ELF_LOAD_OBJ} \
                 ${SYSHD_OPT_OBJ} \
                 ${SWITCH_OBJ} \
                 ${KEYBOARD_INTR_OBJ} \
                 ${STDIO_OBJ} \
                 ${KSTDIO_OBJ}
                 #${SYSCALL_OBJ}


ALL_OBJ = ${DISP_CHAR_OBJ} \
      		${ELF_LOAD_OBJ} \
      		${READKERNEL_OBJ} \
      		${LIB_BIN_TO_BCD_OBJ} \
      		${KPUTX_OBJ} \
      		${DLIST_OBJ} \
      		${BUDDY_OBJ} \
      		${PAGE_OBJ} \
      		${STRING_OBJ} \
      		${INTR_HANDLE_ASM_OBJ} \
      		${IDT_INIT_OBJ} \
      		${PIC_OBJ} \
      		${TIMER_8253_OBJ} \
      		${TIMER_INTR_HANDLE_OBJ} \
      		${INTERRUPT_OBJ} \
      		${DEBUG_OBJ} \
      		${BOOTLOAD_OBJ} \
      		${BIN_TREE_OBJ} \
      		${AVL_TREE_OBJ} \
      		${BITMAP_OBJ} \
      		${HASH_OBJ} \
      		${MM_OBJ} \
      		${PROC_OBJ} \
      		${KERNEL_MM_OBJ} \
      		${SWITCH_OBJ} \
      		${KSYNC_OBJ} \
      		${MSG_OBJ} \
      		${VIDEO_OBJ} \
      		${SYSCALL_OBJ} \
      		${SYSHD_OPT_OBJ} \
      		${KEYBOARD_INTR_OBJ} \
      		${TTY_OBJ} \
      		${KEYBOARD_OBJ} \
      		${STDIO_OBJ} \
      		${KSTDIO_OBJ} \
      		${KERNEL_OBJ}



# 二进制文件
MBR_BIN	= build/mbr.bin
BOOTLOAD_BIN	= build/bootload.bin
KERNEL_BIN = build/kernel.bin

# 服务进程或用户进程
VIDEO_BIN = build/video.bin
TTY_BIN = build/tty.bin
KEYBOARD_BIN = build/keyboard.bin

# 编译目标文件
# 汇编
${DISP_CHAR_OBJ} : ${DISP_CHAR_SRC_FILE} ${DISP_CHAR_DEP_FILE}
	${ASM} ${ASM_INCLUDE_DIR} -f elf32 -o $@ $<
${ELF_LOAD_OBJ} : ${ELF_LOAD_SRC_FILE} ${ELF_LOAD_DEP_FILE}
	${ASM} ${ASM_INCLUDE_DIR} -f elf32 -o $@ $<
${READKERNEL_OBJ} : ${READKERNEL_SRC_FILE} ${READKERNEL_DEP_FILE}
	${ASM} ${ASM_INCLUDE_DIR} -f elf32 -o $@ $<
${INTR_HANDLE_ASM_OBJ} : ${INTR_HANDLE_ASM_SRC_FILE} ${INTR_HANDLE_ASM_DEP_FILE}
	${ASM} ${ASM_INCLUDE_DIR} -f elf32 -o $@ $<
${SWITCH_OBJ} : ${SWITCH_SRC_FILE}
	${ASM} ${ASM_INCLUDE_DIR} -f elf32 -o $@ $<
${SYSHD_OPT_OBJ} : ${SYSHD_OPT_SRC_FILE} ${SYSHD_OPT_DEP_FILE}
	${ASM} ${ASM_INCLUDE_DIR} -f elf32 -o $@ $<


# c语言
${LIB_BIN_TO_BCD_OBJ} : ${LIB_BIN_TO_BCD_SRC_FILE} ${LIB_BIN_TO_BCD_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${KPUTX_OBJ} : ${KPUTX_SRC_FILE} ${KPUTX_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${BOOTLOAD_OBJ} : ${BOOTLOAD_SRC_FILE} ${BOOTLOAD_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${DLIST_OBJ} : ${DLIST_SRC_FILE} ${DLIST_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${BUDDY_OBJ} : ${BUDDY_SRC_FILE} ${BUDDY_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${PAGE_OBJ} : ${PAGE_SRC_FILE} ${PAGE_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${STRING_OBJ} : ${STRING_SRC_FILE} ${STRING_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${IDT_INIT_OBJ} : ${IDT_INIT_SRC_FILE} ${IDT_INIT_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${PIC_OBJ} : ${PIC_SRC_FILE} ${PIC_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${TIMER_8253_OBJ} : ${TIMER_8253_SRC_FILE} ${TIMER_8253_DEF_FILE}
	${CC} ${CFLAGS} -o $@ $<
${TIMER_INTR_HANDLE_OBJ} : ${TIMER_INTR_HANDLE_SRC_FILE} ${TIMER_INTR_HANDLE_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${INTERRUPT_OBJ} : ${INTERRUPT_SRC_FILE} ${INTERRUPT_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${DEBUG_OBJ} : ${DEBUG_SRC_FILE} ${DEBUG_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${BIN_TREE_OBJ} : ${BIN_TREE_SRC_FILE} ${BIN_TREE_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${AVL_TREE_OBJ} : ${AVL_TREE_SRC_FILE} ${AVL_TREE_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${BITMAP_OBJ} : ${BITMAP_SRC_FILE} ${BITMAP_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${HASH_OBJ} : ${HASH_SRC_FILE} ${HASH_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${MM_OBJ} : ${MM_SRC_FILE} ${MM_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${KERNEL_MM_OBJ} : ${KERNEL_MM_SRC_FILE} ${KERNEL_MM_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${PROC_OBJ} : ${PROC_SRC_FILE} ${PROC_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${KSYNC_OBJ} : ${KSYNC_SRC_FILE} ${KSYNC_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${MSG_OBJ} : ${MSG_SRC_FILE} ${MSG_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${KEYBOARD_INTR_OBJ} : ${KEYBOARD_INTR_SRC_FILE} ${KEYBOARD__INTR_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<

${SYSCALL_OBJ} : ${SYSCALL_SRC_FILE} ${SYSCALL_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${VIDEO_OBJ} : ${VIDEO_SRC_FILE} ${VIDEO_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${TTY_OBJ} : ${TTY_SRC_FILE} ${TTY_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${KEYBOARD_OBJ} : ${KEYBOARD_SRC_FILE} ${KEYBOARD_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${STDIO_OBJ} : ${STDIO_SRC_FILE} ${STDIO_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<
${KSTDIO_OBJ} : ${KSTDIO_SRC_FILE} ${KSTDIO_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<

${KERNEL_OBJ} : ${KERNEL_SRC_FILE} ${KERNEL_DEP_FILE}
	${CC} ${CFLAGS} -o $@ $<





# 编译二进制文件
${MBR_BIN} : ${MBR_SRC_FILE} ${MBR_DEP_FILE}
	${ASM} ${ASM_INCLUDE_DIR} -o $@ $<
${BOOTLOAD_BIN} : ${BOOTLOAD_DEP_OBJ}
	${LD} ${BOOTLOAD_LDFLAGS} -o $@ $^
${KERNEL_BIN} : ${KERNEL_DEP_OBJ}
	${LD} ${KERNEL_LDFLAGS} -o $@ $^

# 用户进程或服务进程
${VIDEO_BIN} : ${VIDEO_OBJ} ${VIDEO_DEP_OBJ}
	${LD} ${PROC_LDFLAGS} -o $@ $^
${TTY_BIN} : ${TTY_OBJ} ${TTY_DEP_OBJ}
	${LD} ${PROC_LDFLAGS} -o $@ $^
${KEYBOARD_BIN} : ${KEYBOARD_OBJ} ${KEYBOARD_DEP_OBJ}
	${LD} ${PROC_LDFLAGS} -o $@ $^



# 伪目标
.PHONY : mbr obj bootload kernel clean all test proc

obj : ${ALL_OBJ}


mbr : ${MBR_BIN} ${HD}
	dd if=${MBR_BIN} of=${HD} bs=512 count=1 conv=notrunc
bootload : ${BOOTLOAD_BIN} ${HD}
	dd if=${BOOTLOAD_BIN} of=${HD} seek=2 bs=512  conv=notrunc
kernel : ${KERNEL_BIN} ${HD}
	dd if=${KERNEL_BIN} of=${HD} seek=100 bs=512  conv=notrunc
video : ${VIDEO_BIN} ${HD}
	dd if=${VIDEO_BIN} of=${HD} seek=300 bs=512  conv=notrunc
tty : ${TTY_BIN} ${HD}
	dd if=${TTY_BIN} of=${HD} seek=400 bs=512  conv=notrunc
keyboard : ${KEYBOARD_BIN} ${HD}
	dd if=${KEYBOARD_BIN} of=${HD} seek=500 bs=512  conv=notrunc

all : mbr bootload kernel video tty keyboard

clean :
	rm -f build/*.o build/*.bin


## 测试库函数所使用的代码
#TEST_BIN_CFLAGS = -Wall $(C_INCLUDE_DIR) -g -fno-builtin -W \
#            -Wstrict-prototypes \
#            -Wmissing-prototypes
#TEST_OBJ_CFLAGS = ${TEST_BIN_CFLAGS} -c

#TEST_SRC_FILE = test.c
#TEST_DEP_FILE = lib/common/include/*.h

#TEST_OBJ = build/test.o
#TEST_BIN_TREE_OBJ = build/test_bin_tree.o
#TEST_AVL_TREE_OBJ = build/test_avl_tree.o
#TEST_BITMAP_OBJ = build/test_bitmap.o
#TEST_STRING_OBJ = build/test_string.o
#TEST_DLIST_OBJ = build/test_dlist.o
#TEST_HASH_OBJ = build/test_hash.o
#TEST_DEP_OBJ = ${TEST_HASH_OBJ} ${TEST_DLIST_OBJ}

#TEST_BIN = build/test.bin

#${TEST_OBJ} : ${TEST_SRC_FILE} ${TEST_DEP_FILE}
#	${CC} ${TEST_OBJ_CFLAGS} -o $@ $<
#${TEST_DLIST_OBJ} : ${DLIST_SRC_FILE} ${DLIST_DEP_FILE}
#	${CC} ${TEST_OBJ_CFLAGS} -o $@ $<
#${TEST_HASH_OBJ} : ${HASH_SRC_FILE} ${HASH_DEP_FILE}
#	${CC} ${TEST_OBJ_CFLAGS} -o $@ $<



#${TEST_BIN} : ${TEST_OBJ} ${TEST_DEP_OBJ}
#	${CC} ${TEST_BIN_CFLAGS} -o $@ $^

#test : ${TEST_BIN}
## end 测试库函数所使用的代码
