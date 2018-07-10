#ifndef __TIMER_8253_H
#define __TIMER_8253_H
#include "global_type.h"


#define __IRQ0_FREQUENCY          100U
#define __8253_INPUT_FREQUENCY    1193180U
//初始化计数值太小的话会触发GP异常
#define __8253_COUNTER0_INIT_VAL  ((__8253_INPUT_FREQUENCY)/(__IRQ0_FREQUENCY))
#define __8253_COUNTER0_PORT      0x40U
#define __8253_COUNTER1_PORT      0x41U
#define __8253_COUNTER2_PORT      0x42U
#define __8253_COUNTER0_MODE      (2U << 1)
#define __8253_RW_LATCH           (3U << 4)
#define __8253_CTRL_PORT          0x43U

void frequency_set( uint16_t counter_port, uint8_t rwl, \
                    uint8_t counter_mode, uint16_t counter_val ) ;

void timer_init( void ) ;

#endif