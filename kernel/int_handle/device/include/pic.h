#ifndef __PIC_INIT_H
#define __PIC_INIT_H

// 8259a 端口常数
#define __8259A_ICW1_MASTER_PORT  0x20
#define __8259A_ICW1_SLAVE_PORT   0xa0
#define __8259A_ICW2_MASTER_PORT  0x21
#define __8259A_ICW2_SLAVE_PORT   0xa1
#define __8259A_ICW3_MASTER_PORT  0x21
#define __8259A_ICW3_SLAVE_PORT   0xa1
#define __8259A_ICW4_MASTER_PORT  0x21
#define __8259A_ICW4_SLAVE_PORT   0xa1
#define __8259A_OCW1_MASTER_PORT  0x21
#define __8259A_OCW1_SLAVE_PORT   0xa1
#define __8259A_OCW2_MASTER_PORT  0x20
#define __8259A_OCW2_SLAVE_PORT   0xa0
#define __8259A_OCW3_MASTER_PORT  0x20
#define __8259A_OCW3_SLAVE_PORT   0xa0

// 8259a 命令常数
#define __8259A_CMD_EOI           0x20
#define __8259A_CMD_ICW1          0x11  //级联,边沿触发,需要写入ICW4
#define __8259A_CMD_ICW2_MASTER   0x20  //起始中断号 0x20
#define __8259A_CMD_ICW2_SLAVE    0x28  //起始中断号 0x28
#define __8259A_CMD_ICW3_MASTER   0x04  //IR2接从片
#define __8259A_CMD_ICW3_SLAVE    0x02  //接主片的IR2
#define __8259A_CMD_ICW4          0x01  //8086模式,正常EOI


void pic_init( void ) ;

#endif