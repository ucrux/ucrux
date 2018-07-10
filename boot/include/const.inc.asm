; 实模式下的 1M 内存布局
; 
; | start |  end  |   size  |            comment             |
; |-------|-------|---------|--------------------------------|
; | ffff0 | fffff | 16B     | BIOS entry (jmp f000:e05b)     |
; | f0000 | fffef | 64K-16B | BIOS range address             |
; | c8000 | effff | 160K    | mapping adapter ROM or I/O map |
; | c0000 | c7fff | 32K     | video adapter BIOS             |
; | b8000 | bffff | 32K     | text mode video adapter        |
; | b0000 | b7fff | 32K     | black-white video adapter      |
; | a0000 | affff | 64K     | color video adapter            |
; | 9fc00 | 9ffff | 1K      | extended BIOS data area        |
; | 7e00  | 9fbff | 608K    | can used by us                 |
; | 7c00  | 7dff  | 512B    | MBR                            |
; | 500   | 7bff  | 30K     | can used by us                 |
; | 400   | 4ff   | 256B    | BIOS data area                 |
; | 000   | 3ff   | 1K      | interrupt vector table         |

; 特殊字符编码
CHAR_NUL                          equ         0x00

; BIOS服务中断号
BIOS_SCREEN_SERVICE               equ         0x10
BIOS_DISK_SERVICE                 equ         0x13
BIOS_MEM_SERVICE                  equ         0x15
; BIOS功能号
BIOS_SCREEN_FUN_CLEAR             equ         0x06
BIOS_SCREEN_FUN_SET_CURSOR        equ         0x02
BIOS_SCREEN_FUN_WR_CHAR           equ         0x0a
BIOS_SCREEN_FUN_WR_STR_WITH_CUR   equ         0x1301
BIOS_SCREEN_FUN_ATTR_BLACK_WHITE  equ         0x07
BIOS_DISK_FUN_CHS_READ            equ         0x02
BIOS_GET_MEM_INFO_E820            equ         0xe820
BIOS_GET_MEM_INFO_E801            equ         0xe801
BIOS_GET_MEM_INFO_88              equ         0x88
; BIOS其他魔数
BIOS_SMAP                         equ         0x534d4150


; 屏幕位置
SCREEN_POSITION_BEGIN         equ         0x00
SCREEN_POSITION_END           equ         0x184f
SCREEN_BLANK_LINE_ATTR        equ         0x07
SCREEN_SCROLL_LINES           equ         0x00

; MBR有bios加载到的地址, BIOS初始化结束后, 会使用 jmp 0:0x7c00
MBR_LOADER_ADDR               equ         0x7c00
; MBR代码所使用的栈,这是栈底地址,栈向底地址增长
MBR_STACK_BASE                equ         (MBR_LOADER_ADDR)
; Address Range Descriptor Structure(ARDS)结构缓存
ARDS_BUF                      equ         0x0500
ARDS_SIZE                     equ         20
MEM_SIZE_ADDR                 equ         0x00007000
GDT_PTR_ADDR                  equ         0x00007004

; bootloader文件在硬盘中位置(1024开始)
BOOTLOADER_FILE_DRIVE         equ         0x80
BOOTLOADER_FILE_CYLINDER      equ         0x00
BOOTLOADER_FILE_HEADER        equ         0x00
BOOTLOADER_FILE_SECTOR        equ         0x03
; bootloader文件最大占用硬盘扇区数(25K,每扇区512B)
BOOTLOADER_FILE_MAX_SECTORS   equ         0x35
; 尝试在硬盘中读取文件的最大次数
FILE_MAX_READ_TIMES           equ         0x06
; bootloader文件被加载的地址.文件大小不要操过25K,否则空间不够用   
BOOTLOADER_FILE_LOAD_ADDR     equ         0x0500
; bootloader入口地址,此处为虚拟地址.此处之后的常量用于:已开启保护模式,且开启分页,使用平坦模型
;KERNEL_BEGIN_ADDR             equ         0xc0000000
BOOTLOADER_ENTRY_ADDR         equ         0x00091000
KERNEL_STACK_BASE             equ         0x00007000 ;于bootloader也使用的栈地址相同

; 保护模式相关常数
; 4G 大小的段的 base 和 limit
FLAT_BASE                     equ         0x00
FLAT_LIMIT                    equ         0xfffff
; 显卡段的 base 和 limit
VIDEO_BASE                    equ         0x000b8000
VIDEO_LIMIT                   equ         0xffff
;KERNEL_VIDEO_BASE             equ         (KERNEL_BEGIN_ADDR + VIDEO_BASE)
;KERNEL_VIDEO_LIMIT            equ         (VIDEO_LIMIT)
; TSS段 base 和 limit
TSS_BASE                      equ         0x00007100
TSS_SIZE                      equ         0x68
TSS_LIMIT                     equ         (TSS_SIZE-0x01) ;104 不带 I/O 特权地图
;KERNEL_TSS_BASE               equ         (KERNEL_BEGIN_ADDR + TSS_BASE)
;KERNEL_TSS_LIMIT              equ         (TSS_LIMIT)     

; 页表相关常数
; 如果最大物理内存小于1M(0x00100000),则初始话系统最大内存这么大的物理地址
; 否则最大初始化1M(0x00100000)的物理地址
; 初始化物理地址时,对等映射低最大1M地址;
ADDR_INIT_MAX                 equ         0x00400000
PAGE_BASE_ADDR                equ         0x00008000   ; 1M
PTE_TABLE_0_ADDR              equ         0x00009000   
;PTE_TABLE_3G_ADDR             equ         0x00110000
;PTE_3G_OFFSET                 equ         0x00000c00
PDE_MASK                      equ         0xffc00000
PTE_MASK                      equ         0x003ff000
PTE_SIZE                      equ         0x04

; 默认的eflags值
DEFAUL_EFLAGS                 equ         0x00001002





