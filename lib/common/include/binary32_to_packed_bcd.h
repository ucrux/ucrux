#ifndef __BIN32_TO_PACKED_BCD_H
#define __BIN32_TO_PACKED_BCD_H
#include "global_type.h"

/*
// 8 bits for example
for( i = 0 ; i < 8 ; i++ )
  //check all columns for >=5 
  //每一个 column 是十进制的一位数
  for each column 
    if column >= 5
      column += 3 ;
  //shift all binary digits left 1
  Hundres <<= 1;
  Hundres[0] = Tens[3];
  Tens <<= 1;
  Tens[0] = Ones[3];
  Ones <<= 1;
  Ones[0] = Binary[7];
  Binary <<= 1;
*/

#define __LENGTH_OF_INT 32
#define __LENGTH_OF_INT_BCD 10
#define __COLUMN_SIZE 4

/*本函数的作用,将32位二进制数转化为压缩的BCD码,即一个字节表示两位BCD码*/
uint64_t bin32_to_bcd( uint32_t input ) ;
#endif