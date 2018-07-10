#include "timer_8253.h"
#include "io.h"

#define __8253_COUNTER0_NO    (0U << 6)
#define __8253_COUNTER1_NO    (1U << 6)
#define __8253_COUNTER2_NO    (2U << 6)
#define __8253_RWL_LOCK       (0U << 4)
#define __8253_RWL_LOW        (1U << 4)
#define __8253_RWL_HIGH       (2U << 4)
#define __8253_RWL_16BITS     (3U << 4)




void
frequency_set( uint16_t counter_port, uint8_t rwl, \
               uint8_t counter_mode, uint16_t counter_val )
{
  uint8_t counter_no ;
  switch( counter_port )
  {
    case __8253_COUNTER0_PORT :
      counter_no = __8253_COUNTER0_NO ;
      break ;
    case __8253_COUNTER1_PORT :
      counter_no = __8253_COUNTER1_NO ;
      break ;
    case __8253_COUNTER2_PORT :
      counter_no = __8253_COUNTER2_NO ;
      break ;
    default :
      return ;
  }
  // 写入 8253 控制寄存器
  outb( counter_no | counter_mode | rwl , __8253_CTRL_PORT ) ;
  // 根据rwl模式写入初始值
  switch( rwl ) 
  {
    case __8253_RWL_16BITS :
    case __8253_RWL_LOW :
      outb( (uint8_t)(counter_val & 0xff) ,counter_port ) ;
    case __8253_RWL_HIGH :
      outb( (uint8_t)((counter_val & 0xff00) >> 8), counter_port ) ;
    default :
      return ;
  }
}


void
timer_init( void )
{
  frequency_set( __8253_COUNTER0_PORT, __8253_RW_LATCH, \
                 __8253_COUNTER0_MODE, __8253_COUNTER0_INIT_VAL ) ;
}
