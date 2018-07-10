%include "disp_char_with_color.inc.asm"

[bits 32]
section .text
;导出函数列表
GLOBAL disp_char_with_color


;---------------------------------------------
disp_char_with_color :                                  ;显示一个字符
; 为了让退格和换行可以正常的配合
; 所以让每行的第一个字符都不显示字符
; 如果每行的第一个字符是0x20(空格),即上一行没有使用换行.
; 如果每行的第一个字符是0x00(NULL),即上一行使用了换行符号
;短调用
;栈传递参数,默认显存段选择子gs   
;                   |  颜色代码  |
;    栈顶--------->  |要打印的字符 |
  push   ebp
  mov    ebp, esp
  push   eax
  push   ebx
  push   ecx
  push   edx

  xor    ebx, ebx
  xor    eax, eax
  mov    ecx, [ebp+8]        ;要打印的字符
  mov    edx, [ebp+12]       ;color
  mov    ch, dl

  ;以下取当前光标位置
  GET_SCREEN_CURSOR          ;ax = 当前光标位置
  mov    esi, eax            ;保存光标位置                

; 处理换行符,以0x0a为换行符
.put_lf:                                     
  cmp    cl, CHAR_LF      ;换行符？
  jnz    .put_bs          ;不是，退格
  mov    ebx, SCREEN_LINE_SIZE                      
  div    bl
  mul    bl
  mov    bx, ax
  add    bx, SCREEN_LINE_SIZE
  mov    cl, CHAR_NUL
  WRITE_CHAR_TO_SCREEN
  inc    bx
  jmp    .roll_screen     ;卷屏
  
.put_bs:  
;退格,只是单纯的清空一个字符
;至于是否真的需要清空,由更高级的接口处理
  cmp    cl, CHAR_BS
  jnz    .put_other
  mov    ebx, SCREEN_LINE_SIZE
  div    bl
  mov    ebx, esi
  cmp    ah, 1                ;判断是否在自定义的行首
  jnz    .bs_not_in_column_01 ;不是自定义的行首跳转
  dec    bx
  shl    bx, 1
  mov    ax, [gs:ebx]         ;拿到真正的行首字符,如果是 0x00,那么上一行就使用了换行符
  mov    ebx, esi
  cmp    al, CHAR_NUL
  jnz    .bs_back_line_not_use_lf 
  jmp    .set_cursor
.bs_back_line_not_use_lf:
  dec    bx
.bs_not_in_column_01:
  dec    bx
  mov    cl, CHAR_BLANK
  WRITE_CHAR_TO_SCREEN
  jmp    .set_cursor

.put_other:                    ;正常显示字符
  mov    bl, SCREEN_LINE_SIZE
  div    bl
  mov    ebx, esi
  WRITE_CHAR_TO_SCREEN
  cmp    ah, SCREEN_LINE_SIZE - 1 ;检查是不是在行尾
  jnz    .not_in_column_tail
  inc    bx
  mov    cl, CHAR_BLANK
  WRITE_CHAR_TO_SCREEN
.not_in_column_tail: 
  inc    bx

.roll_screen:
  cmp    bx, 2000         ;光标超出屏幕？滚屏
  jl     .set_cursor
  ROLL_SCREEN

.set_cursor:          
  SET_SCREEN_CURSOR       ;设置光标,输入bx=要保存的光标位置

.put_done :
  pop    edx
  pop    ecx
  pop    ebx
  pop    eax
  pop    ebp

  ret
;------------------------------

