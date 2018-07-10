#include "pic.h"
#include "io.h"

void 
pic_init( void )
{
  // 初始化8259a主片
  outb( __8259A_CMD_ICW1, __8259A_ICW1_MASTER_PORT ) ;
  outb( __8259A_CMD_ICW2_MASTER, __8259A_ICW2_MASTER_PORT ) ;
  outb( __8259A_CMD_ICW3_MASTER, __8259A_ICW3_MASTER_PORT ) ;
  outb( __8259A_CMD_ICW4, __8259A_ICW4_MASTER_PORT ) ;

  // 初始化8259a从片
  outb( __8259A_CMD_ICW1, __8259A_ICW1_SLAVE_PORT ) ;
  outb( __8259A_CMD_ICW2_SLAVE, __8259A_ICW2_SLAVE_PORT ) ;
  outb( __8259A_CMD_ICW3_SLAVE, __8259A_ICW3_SLAVE_PORT ) ;
  outb( __8259A_CMD_ICW4, __8259A_ICW4_SLAVE_PORT ) ;

  // 开启时钟中断,键盘中断,屏蔽其他中断,ATA0的中断(硬盘)
  outb( 0xf8, __8259A_OCW1_MASTER_PORT ) ;
  outb( 0xbf, __8259A_OCW1_SLAVE_PORT ) ;
}

