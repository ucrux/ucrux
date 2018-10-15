本内核为一个微内核,仅适用于单核单线程的x86,32位cpu<br>
本内核仅能从BIOS启动<br>
内核线性地址空间从 0-1G,用户地址空间从 1G-4G

测试编译环境
===

### gcc(x64)

```
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-linux-gnu/5/lto-wrapper
Target: x86_64-linux-gnu
Configured with: ../src/configure -v --with-pkgversion='Ubuntu 5.4.0-6ubuntu1~16.04.10' --with-bugurl=file:///usr/share/doc/gcc-5/README.Bugs --enable-languages=c,ada,c++,java,go,d,fortran,objc,obj-c++ --prefix=/usr --program-suffix=-5 --enable-shared --enable-linker-build-id --libexecdir=/usr/lib --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --with-sysroot=/ --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-gnu-unique-object --disable-vtable-verify --enable-libmpx --enable-plugin --with-system-zlib --disable-browser-plugin --enable-java-awt=gtk --enable-gtk-cairo --with-java-home=/usr/lib/jvm/java-1.5.0-gcj-5-amd64/jre --enable-java-home --with-jvm-root-dir=/usr/lib/jvm/java-1.5.0-gcj-5-amd64 --with-jvm-jar-dir=/usr/lib/jvm-exports/java-1.5.0-gcj-5-amd64 --with-arch-directory=amd64 --with-ecj-jar=/usr/share/java/eclipse-ecj.jar --enable-objc-gc --enable-multiarch --disable-werror --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --with-tune=generic --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu
Thread model: posix
gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.10)
```

### nasm(x64)
```
NASM version 2.11.08
```

### ld(x64)
```
GNU ld (GNU Binutils for Ubuntu) 2.26.1
```

### bochs
```
version 2.6.9
```

总体说明
===
- 为了实现的简便性,内核空间的代码不可重入(减少同步互斥机制)
- 基础内核数据全在最低1M内存中
- 内核空间的内存模型为平坦模型,也就是说内核不能使用高于1G的物理内存
- 内核主要组件
  - 内存管理
    - 本内核最大支持1G的物理内存
  - 中断处理
  - 进程的生命周期管理及调度
  - 消息的发送及接收
  - 驱动程序注册
  - 文件系统

- 本内核将使用两块硬盘
  - 第一块硬盘仅存储MBR及以固定位置存储内核,不包含任何文件系统
  - 第二块硬盘将实现一个简陋的文件系统

```
第一块硬盘文件存储位置地图

0 - 511 B       MBR_BIN
1024 - *        BOOT_LOADER_FILE
```

```
MBR及内核加载进内存后的memory map

0x0500          BOOT_LOADER elf文件地址
0x7c00          MBR_BIN,GDT等重要数据结构

# 开启保护模式后
0x00007000          内存总大小,内核初始化栈
0x00007004          GDT_PTR
0x00007100          TSS段
0x00008000          初始化页目录表
0x00009000          内核初始化页表项表
0x00011000          内核入口地址
0x00050000          内核elf文件地址,重定向内核后就没有用了
0x00091000          bootloader入口,加载内核之后就没用了
```


全局描述符表
===

- 内核将使用使用6个全局段描述符
  - 空描述符,x86CPU规定
  - 内核代码段,访问整个4G内存
  - 内核数据段,访问整个4G内存
  - 显存段,访问b8000-bffff
  - TSS描述符,用于任务切换
  - 局部描述符表描述符,各进程或线程使用

mbr
===

- 初始化屏幕
- 初始化GDT
- 进入保护模式
- 初始化TSS
- 初始化页目录表
- 初始化页表项表
- 加载bootloader

bootloader
===

- 加载内核

文件系统
===
- 主盘之存放操作系统内核,和一些必要的操作系统服务
- 一个异步的磁盘驱动
  - 通过中断和消息机制实现
- 文件系统结构的设计

## 使用fdisk对从盘进行分区(hdb.vhd)

- 硬盘容量 = 单片容量 X 磁头数 
- 单片容量 = 每磁道扇区数 X 磁道数 X 512byte
- 磁道数又等于柱面数,因此
  - 硬盘容量 = 每磁道扇区数 x 柱面数 x 512 x 磁头数

> 需要使用fdisk的扩展功能x输入柱面数和磁头数

**分区表64个字节,每个表项16字节,必须存在于MBR或EBR中.在这512字节中,前446字节事硬盘参数和引导程序,然后才是64字节的分区表,最后2字节事魔数55aa**

- 主引导记录MBR
- 磁盘分区表DPT
- 结束魔数55AA

## 分区表项机构

| offset | (bytes) |                  comment                  |
|--------|---------|-------------------------------------------|
|      0 |       1 | 0x80:active paritition; 0x00:non-loadable |
|      1 |       1 | partition begin header                    |
|      2 |       1 | partition begin sector                    |
|      3 |       1 | partition begin cylinder                  |
|      4 |       1 | file system id                            |
|      5 |       1 | partition end header                      |
|      6 |       1 | partition end sector                      |
|      7 |       1 | partition end cylinder                    |
|      8 |       4 | partition begin sector offset             |
|     12 |       4 | partition all sectors                     |


to do list
===

**现阶段的工作告一段落吧,要做正事了**

- 完善tty(shell)(哈哈,shell 和 tty 合体了).
- 完成硬盘驱动
- 完成文件系统


附录
===

## IA32指令格式
```
前缀  操作码  寻址方式,操作数类型  立即数  偏移量
```

## BIOS功能简介


### 清屏

```asm
; 清屏利用 0x06 号, 上卷全部行, 则可清屏
; --------------------------------------
; int 0x10 功能号:0x06  功能描述:上卷窗口
; --------------------------------------
; 输入:
; AH 功能号 = 0x06
; AL = 上卷的行数(如果为0,表示全部)
; BH = 上卷行属性
; (CL,CH) = 窗口左上角的(X,Y)位置
; (DL,DH) = 窗口右下角的(X,Y)位置
; 输出: 无
  mov    ax, 0x0600
  mov    bx, 0x0700
  mov    cx, 0         ; 左上角: (0,0)
  mov    dx, 0x184f    ; 右下角: (80,25)
                       ; VGA文本模式中,一行能容纳80个字符,共25行
                       ; 下标从0开始,所以 0x18=24, 0x4f=79
  int    0x10
```

### 设置光标位置
```asm
; --------------------------------------
; int 0x10 功能号:0x02  功能描述:设置光标位置
; --------------------------------------
; 输入:
; AH 功能号 = 0x02
; BH = 页码
; DH = 列
; DL = 行
  mov   ax, 0x0200
  mov   bx, 0x0000
  mov   dx, 0x0000
  int   0x10
```

### 在光标处写入字符
```asm
; --------------------------------------
; int 0x10 功能号:0x02  功能描述:设置光标位置
; --------------------------------------
; 输入:
; AH 功能号 = 0x0a
; BH = 页码
; AL = 字符
; CX = 打印字符的次数
  mov    ah, 0x0a
  mov    al, 'c'
  mov    bh, 0x00
  mov    cx, 1
  int    0x10
```

### 获取光标位置
```asm
; --------------------------------------
; int 0x10 功能号:0x03 功能描述:获取光标位置
; --------------------------------------
; 输入:
; AH 功能号 = 0x03
; BH = 待获取光标的页号
; 输出:
; ch = 光标开始行
; cl = 光标结束行
; dh = 光标所在行
; dl = 光标所在列
  mov    ah, 0x03
  mov    bh, 0x00     ; bh寄存器存储的是待获取光标的页号
  int    0x10
```

### 打印字符串
```asm
; --------------------------------------
; int 0x10 功能号:0x13 功能描述:打印字符串
; --------------------------------------
; 输入:
; AH 功能号 = 0x13
; AL = 显示字符的光标属性
; BH = 要显示字符的页号
; BL = 字符属性
; CX = 字符串的长度(不包含结束符)
; DH = 光标所在行
; DL = 光标所在列
; ES:BP = 字符串首地址
; 输出: 无
  mov    ax, ds
  mov    es, ax
  mov    dx, 0x00      ; 从0行0列打印
  mov    bp, msg_addr
  mov    cx, msg_len
  mov    ax, 0x1301    ; al=0x01:显示字符串,光标跟随移动
  mov    bx, 0x02      ; bl=0x02:黑底绿字
  int    0x10
```

### 读取硬盘
```asm
; --------------------------------------
; int 0x13 功能号:0x02 功能描述:读取硬盘
; --------------------------------------
; 输入:
;    AH = 02
;    AL = number of sectors to read  (1-128 dec.)
;    CH = track/cylinder number  (0-1023 dec., see below)
;    CL = sector number  (1-17 dec.)
;    DH = head number  (0-15 dec.)
;    DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
;    ES:BX = pointer to buffer
;输出:
;    AH = status  (see INT 13,STATUS)
;    AL = number of sectors read
;    CF = 0 if successful
;       = 1 if error
;其他说明:
;    - BIOS disk reads should be retried at least three times and the
;      controller should be reset upon error detection
;    - be sure ES:BX does not cross a 64K segment boundary or a
;      DMA boundary error will occur
;    - many programming references list only floppy disk register values
;    - only the disk number is checked for validity
;    - the parameters in CX change depending on the number of cylinders;
;      the track/cylinder number is a 10 bit value taken from the 2 high
;      order bits of CL and the 8 bits in CH (low order 8 bits of track):
;
;      |F|E|D|C|B|A|9|8|7|6|5-0|  CX
;       | | | | | | | | | |  `-----  sector number
;       | | | | | | | | `---------  high order 2 bits of track/cylinder
;       `------------------------  low order 8 bits of track/cyl number

  mov    ax, ds
  mov    es, ax
  mov    ah, 0x02
  mov    al, 0x10
  mov    ch, 0x00
  mov    cl, 0x01
  mov    dh, 0x00
  mov    dl, 0x80
  mov    bx, 0x7e00
  int    0x13
```


### 获取内存布局
#### 0xe820
```asm
; Address Range Descriptor Structure
; | offset |  attr name   |         comment         |
; |--------|--------------|-------------------------|
; |      0 | BaseAddrLow  | base addr low 32 bits   |
; |      4 | BaseAddrHigh | base addr high 32 bits  |
; |      8 | LengthLow    | mem length low 32 bits  |
; |     12 | LengthHigh   | mem length high 32 bits |
; |     16 | Type         | type of this mem        |
; Type的取值
; | type's value |         name         |      comment       |
; |--------------|----------------------|--------------------|
; | 1            | AddressRangeMemory   | can used by OS     |
; | 2            | AddressRangeReserved | Reserved,can't use |
; | other        | undefined            | can't use          |
;
; ---------------------------------------
; int 0x15 功能号:0xe820 功能描述:获取内存布局
; ---------------------------------------
; 输入:
; EAX 子功能号 = 0xe820
; EBX = ARDS后续值,第一次调用时一定要设置为0,每次中断返回,BIOS会更新此值
; ES:DI = ARDS缓冲区,BIOS将获取的内存值写入此寄存器指向的内存,每次都以ARDS格式返回
; ECX = ARDS结构的字节大小,用来指示BIOS写入的字节数,20字节
; EDX = 固定为签名标记 0x534d4150, 为 SMAP 的 ASCII 码
;
; 输出:
; CF = 0 正确, 1 出错
; EAX = 0x534d4150
; ES:DI = 同输入值
; ECX = 向ARDS写入的字节数, BIOS最小写入20字节
; EBX = ARDS后续值,如果CF=0,EBX=0,则这是ARDS最后一个结构
.get_mem_array_e820:
  xor    ebx, ebx
  mov    di, ards_buf
  mov    edx, 0x534d4150
  mov    eax, 0xe820
  mov    ecx, 20
  int    0x15
  jc     .get_mem_array_err
  add    di,  ecx
  cmp.   ebx, 0
  jnz    .get_mem_array
```

#### 0xe801
```asm
; 最大识别4GB内存,低于15M的内存以1K为单位大小来记录
; 单位数量在寄存器AX和CX中记录,其中AX和CX的值是一样的
; 所以在15M空间以下的实际内存容量=AX*1024,AX和CX的最大值为0x3c00
; 16MB~4GB是以64K为单位来记录的,单位数量在寄存器BX和DX中(BX==DX)
; 所以16M以上的空间实际大小=BX*64*1024
; ---------------------------------------
; int 0x15 功能号:0xe801 功能描述:获取内存布局
; ---------------------------------------
; 输入:
; AX 功能号 = 0xE801
;
; 输出:
; CF = 0 正确, 1 出错
; AX = 以 1K 为单位的内存数量(15M以下)
; BX = 以 64K 为单位的内存数量( 16M ~ 4G)
; CX == AX
; DX == BX
.get_mem_array_e801:
  mov  ax, 0xe801
  int  0x15
  jc   .get_mem_array_err
```

#### 0x88
```asm
; 最大识别64M内存,只显示1M以上的内存
; ---------------------------------------
; int 0x15 功能号:0x88 功能描述:获取内存布局
; ---------------------------------------
; 输入:
; AH 功能号 = 0x88
; CF = 0 正常, 1 出错
; 输出:
; AX = 以 1K 为单位,内存空间1M以上的单位数量(内存大小 = AX*1024+1M)
.get_mem_array_88
  mov    ah, 0x88
  int    0x15
```

## ELF文件简介

### 字段类型定义

| type name  | bytes | align | comment |
|------------|-------|-------|---------|
| Elf32_Half |     2 |     2 |         |
| Elf32_Word |     4 |     4 |         |
| Elf32_Addr |     4 |     4 | address |
| Elf32_Off  |     4 |     4 | offset  |

```c
//ELF header 结构
struct Elf32_Ehdr
{
    unsigned char e_ident[16] ; //魔数
    Elf32_Half  e_type ;        //elf文件类型
    Elf32_Half  e_machine ;     //机器类型
    Elf32_Word  e_version ;     //版本信息
    Elf32_Addr  e_entry ;       //程序入口
    Elf32_Off   e_phoff ;       //程序头表在文件内的字节偏移量
    Elf32_Off   e_shoff ;       //节头表在文件内的字节偏移量
    Elf32_Word  e_flags ;       //指名于处理器相关的标志
    Elf32_Half  e_ehsize ;      //指明elf header的字节大小
    Elf32_Half  e_phentsize ;   //指明程序头表中每个条目的字节大小
    Elf32_Half  e_phnum ;       //指明程序头表的数量
    Elf32_Half  e_shentsize ;   //指明节头表每个条目的字节大小
    Elf32_Half  e_shnum ;       //节头表中条目的数量
    Elf32_Half  e_shstmdx ;     //指明string name table在节头表中的索引
} ;

//程序头结构
struct Elf32_Phdr
{
    Elf32_Word      p_type ;    //指明改段的类型
    Elf32_Off       p_offset ;  //指明本段在文件内的起始偏移字节
    Elf32_Addr      p_vaddr ;   //指明本段在内存中的其实虚拟地址
    Elf32_Addr      p_paddr ;   //仅用于与物理地址相关的系统中
    Elf32_Word      p_filesz ;  //指明本段在文件中的大小
    Elf32_Word      p_memsz ;   //本段在内存中的大小
    Elf32_Word      p_flags ;   //相关标志
    Elf32_Word      p_align ;   //文件和内存中的对齐方式
}
```

## 内联汇编
### AT&T汇编语法简介

- 指令后面加上了大小后缀
  - b 1字节 (8 bits)
  - w 2字节 (16 bits)
  - l 4字节 (32 bits)
- 寄存器前面有前缀 % (%eax)
- 源操作数在左,目标操作数在右( mov eax, ebx == movl %ebx, %eax)
- 立即数有前缀 $ ($6)
- 远跳转 ljmp $segmentL$offset
- 远调用 lcall $segment:$offset
- 远返回 lret $n
- 不带前缀的立即数就是内存地址

### AT&T内存寻址
```S
segreg(段基址):base_address(offset_address,index,size)
; 该格式对应表达式为
segreg(段基址):base_address + offset_address + index*size
; 对应intel语法
segreg:[base + index*size + offset]
```

- base_address : 基地址;可以为整数,变量名,可正可负
- offset_address : 偏移地址,必须是8个通用寄存器之一
- index : 索引值;必须是8个通用寄存器之一
- size : 单位长度,只能是1,2,4,8

#### 寻址方式

- 直接寻址(只有base_address)
  - movl $255, 0xc00008f0 把255拷贝到0xc00008f0这个地址中
  - mov $255, var 把255拷贝到var这个变量的地址中
- 寄存器间接寻址(只有offset_address)
  - movl (%ebx), %eax, 把ebx的值对应的地址中的值拷贝到eax中
- 寄存器相对寻址(base_address,offset_address)
  - movb -4(%ebx), %al, 将地址 ebx-4 所指向的值拷贝到al中
- 变址寻址
  - movl %eax, (,%esi,2)
    - 将 eax 的值写入 esi*2 所指向的内存
  - movl %eax, (%ebx,%esi,2)
    - 将 eax 的值写入 ebx+esi*2 所指向的内存
  - movl %eax, base_value(,%esi,2)
    - 将 eax 的值写入 base_value+esi*2 所指向的内存
  - movl %eax, base_value(%ebx,%esi,2)
    - 将 eax 的值写入 base_value+ebx+esi*2 所指向的内存

### 基本内联汇编
```c
asm [volatile] ("assembly code")
//volatile 告诉gcc不要优化此汇编
```

#### assembly code规则

1. 指令必须用双引号引起来,无论双引号是一条指令或者多条指令
2. 一对双引号不能跨行,如果跨行结尾用反斜杠'\'转义
3. 指令之间用';',换行符'\n'或换行加制表符'\n\t'分隔

```c
asm  volatile ("movl %9 %eax;""pushl %eax")
```

### 扩展内联汇编
```c
asm [volatile] ("assembly code" : output : input : clobber/modify)
```

- output
  - "操作数修饰符 约束名"(C变量名)
  - 操作数修饰符通常为'='
  - 多个操作数之间用','分隔
- input
  - "[操作数修饰符] 约束名" (C变量名)
  - 操作数修饰符可选
  - 多个操作数之间用','分隔
- clobber/modify
  - 通知编译器可能造成寄存器或内存数据的破坏

#### 寄存器约束

- a: eax/ax/al
- b: ebx/bx/bl
- c: ecx/cx/cl
- d: edx/dx/dl
- D: edi/di
- S: esi/si
- q: 任意4个通用寄存器: eax/ebx/ecx/edx
- r: 任意6个通用寄存器: eax/ebx/ecx/edx/esi/edi
- g: 相当于除了同q一样,还可以安排在内存中
- A: 把eax和edx组成64位整数
- f: 表示浮点寄存器
- t: 表示第一个浮点寄存器
- u: 表示第二个浮点寄存器

```c
#include <stdio.h>
void main( )
{
  int in_a = 1, in_b = 2, out_sum ;
  asm("addl %%ebx, eax" : "=a"(out_sum) : "a"(in_a), "b"(in_b) ) ;
  printf( "sum is %d\n", out_sum ) ;
}
```

output中的'='号是操作数类型修饰符,表示只写

#### 内存约束

- m: 任意一种内存形式
- o: 内存变量,但访问它是通过偏移量的形式访问,即包含offset_address的格式

```c
#include <stdio.h>
void main( )
{
  int in_a = 1, in_b = 2 ;
  printf( "in_b is %d\n", in_b ) ;
  asm( "movb %b0, %1;" : : "a"(in_a), "m"(in_b) ) ;
  printf( "in_b now is %d\n", in_b ) ;
}
```

- "m"(in_b),告诉gcc把变量in_b的**指针**作为内联代码的操作数
- %1,表示input,output中的每一个项的编号,例如 "a"(in_a)为%0
- %b0,表示32位数据的低8位,这里指的是al寄存器


#### 立即数约束

*只能放在input中*

- i: 整数立即数
- F: 浮点数立即数
- I: 0~31之间的立即数
- J: 0~63之间的立即数
- N: 0~255之间的立即数
- O: 0~32之间的立即数
- X: 任何类型立即数

#### 通用约束

- 0~9: 此约束只能用在input部分,但表示可与output和input中第n个操作数相同的寄存器或内存

#### 占位符
##### 序号占位符

对在output和input中的操作数,按照他们从左到右出现的次序从0开始编号,一直到9.应用格式为:%0~%9

```c
asm ( "addl %%ebx, %%eax" : "=a"(out_sum) : "a"(in_a), "b"(in_b) )
```
等价于
```c
asm ( "addl %2, %1" : "=a"(out_sum) : "a"(in_a), "b"(in_b) )
// 其中
// "=a"(out_sum)序号为0, %0对应的是eax
// "a"(in_a)序号为1, %1对应eax
// "b"(in_b)序号为2, %2对应ebx
```


##### 名称占位符

```
[名称] "约束名" (C变量)

引用名称时使用%[名称]
```

```c
#include <stdio.h>
void main( )
{
  int in_a = 18, in_b = 3, out = 0 ;

  asm ( "divb %[divisor]; movb %b1, %[result]" \
        : [result]"=m"(out) \
        : "a"(in_a), [divisor]"m"(in_b) \
      ) ;
}
```


#### 操作数类型修饰符

- output
  - =: 表示操作数只写 "=a"(c_var)  c_var=eax
  - +: 表示操作数可读写,告诉gcc所约束的寄存器或内存先被读入,在被写入
  - &: 表示此output中的操作数要独占所约束(分配)的寄存器,只供output使用,任何input中所分配的寄存器不能与此相同
    - 当表达式中有多个修饰符时,&要与约束名挨着,不能分隔
- input
  - %: 该操作数可以和下一个输入操作数互换

```c
asm ( "addl %1, %0" : "+a"(in_a) : "b"(in_b) ) ;
// in_a += in_b
```
 
```c
#include <stdio.h>
void main( )
{
  int in_a = 1, sum = 0 ;
  asm( "addl %1, %0;" : "=a"(sum) : "I"(2), "0"(in_a) ) ;
  printf( "sum is %d\n", sum ) ;
}
//"%I"(2) 表示对应的操作数可以和下一个输入所约束的操作数对换位置
//"0"(in_a) 要求gcc把分配给C变量in_a的操作数(寄存器或内存)同序号0对应的汇编操作数一样
//  此例中序号0是"=a"(sum),即eax,所以in_a也被分配为eax
```


#### clobber/modify
如果在output,input中通过寄存器约束指定了寄存器,gcc必然会知道这些寄存器会被修改

**在clobber/modify部分明确指明修改了那些寄存器,内存. 多个寄存器之间用逗号','分隔**

```c
asm ("movl %%eax, %0; movl %%eax, %%ebx" : "=m"(ret_value)::"bx","ax")
//只需要声明寄存器的一部分就行了,如:bl,bx,ebx; al,ax,eax
```

- eflags改变 : 用"cc"声明
- 内存改变 : 用"memory"声明,这个声明可以清除寄存器缓存

*C语言中volatile关键字,用于声明常变化的变量,提示编译器每次使用该值时都要从内存中取*

#### 内联汇编机器模式简介
机器模式用来在机器层面上地丁数据的大小及格式

- h: 如:ah,bh,ch,dh
- b: 如:al,bl,cl,dl
- w: 如:ax,bx,cx,dx等
- k: 如:eax,ebx,ecx,edx等

## 伙伴系统简介

### 分配叶帧

- 将相同大小的内存块链表组织在相同的链表
- 每个内存块是由2的幂次个页帧组成
- 不同大小的块在空间上不会有重叠
- 当一个需求为4个连续页面时
  - 检查是否有4个页面的空闲块而快速满足请求
    - 若该链表上(每个结点都是大小为4页面的块)有空闲的块,则分配给用户
    - 否则,向下一个级别(order)的链表中查找
      - 若存在(8页面的)空闲块(现处于另外一个级别的链表上)
        - 则将该页面块分裂为两个4页面的块
          - 一块分配给请求者
          - 另外一块加入到4页面的块链表中
      - 否则,继续向下一个级别查找

```
 order       free_area_t
           zone->free_area
   |       ---------------        
   |      |       0       | 
   |       ---------------
   |      |       1       |
   |       ---------------
   |      |       2       | --> | 4 pages |
   |       ---------------
   |      |       3       | --> | 8 pages | 
   |       ---------------
   |      |   MAX ORDER   |
   V       ---------------
```

### 释放叶帧

以上过程的逆过程就是页框块的释放过程,也是该算法名字的由来.内核试图把大小为b的一对空闲伙伴块合并为一个大小为2b的单独块.满足以下条件的两个块称为伙伴:

- 两个块具有相同的大小,记作b
- 它们的物理地址是连续的
- 第一块的第一个页框的物理地址是2×b×2^12的倍数


## 中断发生时的压栈

**仅讨论32位保护模式**

### 如果出现特权等级转移
#### 在旧栈中压入
```
    |   0   | SS_old |   SS_old:esp_old
    |    esp_old     | <------------------
```

#### 转移到相应特权等级的栈

- 将旧栈的内容自动拷贝过来

*如果没有发生特权等级转移,则不会更换新栈,旧栈的内容也如下所示(没有 SS_old和esp_old)*

```
    |   0   | SS_old |
    |    esp_old     |
    |    EFLAGS      |
    |   0   | CS_old |
    |    EIP_old     |  SS_new:esp_new
    |   ERROR_CODE   | <-----------------

ERROR_CODE 不是每种中断都有的
```

- 从中断中返回
  - 如果中断中有错误码,需要手动跳过,再用 iret 
  - iret指令
    - 依次弹出 EIP,CS,EFLAGS
    - 如果发现特权等级变化
      - 继续弹出 ESP, SS

## 中断与异常表

|  No.   | Mnemonic |         descripton         |    type    | err_no |
|--------|----------|----------------------------|------------|--------|
| 0      | #DE      | Divide Error               | fault      | N      |
| 1      | #DB      | debug                      | fault/trap | N      |
| 2      | /        | NMI intr                   | Interrupt  | N      |
| 3      | #BP      | breakpoint                 | Trap       | N      |
| 4      | #OP      | overflow                   | trap       | N      |
| 5      | #BR      | BOUND Range Exceeded       | Fault      | N      |
| 6      | #UD      | invalid Opcode             | Fault      | N      |
| 7      | #NM      | device not avaliable       | Fault      | N      |
| 8      | #DF      | Double fault               | abort      | Y      |
| 9      | #MF      | coprocessor Seg Overrun    | fault      | N      |
| 10     | #TS      | invalid TSS                | fault      | Y      |
| 11     | #NP      | seg Not present            | fault      | Y      |
| 12     | #SS      | stack seg fault            | Fault      | Y      |
| 13     | #GP      | general protection         | fault      | Y      |
| 14     | #PF      | Page fault                 | fault      | Y      |
| 15     |          | Reserved                   |            |        |
| 16     | #MF      | floating-point Err         | fault      | N      |
| 17     | #AC      | alignment Check            | fault      | Y      |
| 18     | #MC      | Machine Check              | Abort      | N      |
| 19     | #XM      | SIMD Float-point Exception | fault      | N      |
| 20-31  |          | reserved                   |            |        |
| 32-255 |          | maskable Interrupts        | Interrupt  |        |

## 可编程中断控制器8259A简介

```
            master 8259
            ---------
            | IRQ00 | 时钟
            | IRQ01 | 键盘
            | IRQ02 | <------------
            | IRQ03 | 串口2        |
            | IRQ04 | 串口1        |
            | IRQ05 | 并口2        |
            | IRQ06 | 软盘         |
            | IRQ07 | 并口1        |
            ---------             |
                                  | 
            slave 8259            |
            ----------------------
            | IRQ08 | 实时时钟
            | IRQ09 | 重定向的IRQ02
            | IRQ10 | 保留
            | IRQ11 | 保留
            | IRQ12 | PS/2 鼠标
            | IRQ13 | FPU异常
            | IRQ14 | 硬盘
            | IRQ15 | 保留
            ---------

```

- INT: 选出优先级最高的中断请求,发送给CPU
- INTA: 中断响应信号,接收来自CPU的中断响应信号
- IMR: 中断屏蔽寄存器,8bits,屏蔽外设的中断
- IRR: 中断请求寄存器,8bits,此寄存器中全是等待处理的中断
- PR: 优先级仲裁寄存器
- ISR: 中断服务寄存器,8bits,当某个中断正在被处理时,保存在此寄存器中

**CPU接收到中断后,中断向量乘以8再加上中断描述符表的基地址,就是我们要的中断描述符**

### 初始化8259a

- initialization command words (ICW)
  - 写入次序为 ICW1 ICW2 ICW3 ICW4
  - 作用
   - 是否级联
   - 设置起始中断向量号
   - 设置中断结束模式
  - ICW1(master:0x20,slave:0xa0)
    - 连接方式
      - 单片工作
      - 级联
    - 中断的触发方式
      - 电平触发
      - 边沿触发 
    - 0 0 0 1 LTIM ADI SNGL IC4
      - IC4:1,写入ICW4;0,不写入ICW4.x86的IC4必须为1
      - SNGL:1,表示单片;0,级联
      - ADI:用来设置8085的调用时间间隔.x86不需要设置
      - LTIM:0,边沿触发;1,电平触发 
  - ICW2(master:0x21,slave:0xa1)
    - 设置起始中断向量号
    - T7 T6 T5 T4 T3 ID2 ID1 ID0
      - 只需写入T3~T7,ID0~ID2都为0.这样起始中断号都是8的倍数
  - ICW3(master:0x21,slave:0xa1)
    - 仅在级联方式下才需要,设置主片和从片用哪个IRQ接口互联
    - 主片
      - S7 S6 S5 S4 S3 S2 S1 S0
        - 置1的那一位对应的IRQ接口用于连接从片
    - 从片
      - 0 0 0 0 0 ID2 ID1 ID0
        - ID0~ID2 即连接主片的IRQ号
  - ICW4(master:0x21,slave:0xa1)
    - 0 0 0 SFNM BUF M/S AEOI uPM
      - SFNM:0,全嵌套模式;1,特殊嵌套模式
      - BUF:0,非缓冲模式;1,缓冲模式
        - 多个8259A级联时,如果工作在缓冲模式下
          - M/S:0,表示本片时从片;1,表示本片是主片
      - AEOI:0,非自动结束中断;1,自动结束中断
      - uPM:0,8080或8085处理器;1,x86处理器 
- operation command word (OCW)
  - OCW发送顺序不固定,3个之中先发送哪个都可以
  - 作用
    - 中断屏蔽
    - 中断结束
  - OCW1(master:0x21;slave:0xa1)
    - 屏蔽外部中断信号 
    - m7 m6 m5 m4 m3 m2 m1 m0
      - m0~m7对应IRQ0~IRQ7.某位为1,对应的IRQ上的中断信号就屏蔽了
  - OCW2(master:0x20;slave:0xa0)
    - 设置中断结束方式
    - 优先级模式
    - R SL EOI 0 0 L2 L1 L0
      - SL:是否指定优先级 
        - 1,可以用OCW2的低3位(L0~L2)来指定ISR寄存器中的哪一个中断被终止
      - R: 0,固定优先级;1,循环优先级,优先级会在0~7内循环
      - EOI:中断结束命令位.1,令ISR寄存器中的相应位清0
        - 如果中断来自主片,只需要向主片发送EOI;如果来自从片,先向从片发送,再向主片发送
      - L0~L2
        - 用于EOI,表示被中断的优先级别
        - 用于优先级循环,指定起始最低优先级
  - OCW3(master:0x20;slave:0xa0)
    - 设置特殊屏蔽方式及查询方式
    - / ESMM SMM 0 1 P PR RIS
      - ESMM,SMM: 启用或禁用特殊屏蔽模式
      - P: 查询命令.1:中断查询模式
      - RR: 读取寄存器命令
      - RIS: 读取中断寄存器选择位.1,选择读取ISR;0,选择读取IRR

## 可编程计数器/定时器8253简介

- 一共有三个计数器
  - 每个计数器由三个部件组成
    - 16bits 计数初值寄存器
    - 16bits 计数器执行部件
    - 16bits 输出锁存器
  - 每个计数器有三个引脚
    - CLK: 时钟输入信号
    - GATE: 门控输入信号
    - OUT: 计数器输出信号
  - 计数器0
    - 端口 0x40
    - 作用 产生实时时钟信号,工作方式3
  - 计数器1
    - 端口 0x41
    - 作用 DRAM定时刷新控制
  - 计数器2
    - 端口 0x42
    - 作用 用于内部扬声器发出不同音调的声音

### 8253控制字

- 操作端口 0x43
- SC1 SC0 RW1 RW0 M2 M1 M0 BCD
  - SC0-SC1
    - 00 选择寄存器0
    - 01 选择寄存器1
    - 10 选择寄存器2
    - 11 未定义
  - RW0-RW1
    - 00 锁存数据,供cpu读
    - 01 只读写低字节
    - 10 只读写高字节
    - 11 先读写低字节,后读写高字节
  - M0-M2
    - 000 工作方式0 计数结束中断方式
    - 001 工作方式1 吋间可重触发单稳方式
    - X10 工作方式2 比率发生器
    - X11 工作方式3 方波发生器
    - 100 工作方式4 软件出发选通
    - 101 工作方式5 硬件触发选通
  - BCD
    - 0 二进制
    - 1 BCD码 

**三个计数器的工作频率都是1.19318Mhz**

1193180 / 计数器0的初始计数值 = 中断信号的频率
1193180 / 中断信号的频率 = 计数器0的初始计数值

## 应用程序二进制接口(ABI)
### 一下五个寄存器在函数调用前后不能被改变

- ebp
- ebx
- edi
- esi
- esp

```
      | ARGn     |
      | ARGn-1   |
      | ARGn-2   |
      | ...      |
      | ARG1     |
      | ret addr | <--- esp
      |          |
```


