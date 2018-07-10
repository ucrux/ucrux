#include "binary32_to_packed_bcd.h"


/*本函数的作用,将32位二进制数转化为压缩的BCD码,即一个字节表示两位BCD码*/
uint64_t
bin32_to_bcd( uint32_t input )
{
  uint64_t ret = 0, adjust, mask_adjust=0x0f, mask_bcd ;
  int32_t bin_loop, bcd_loop ;
  uint32_t mask_h = 0x80000000, shift_bits, tmpin ;

  for( bin_loop = 0 ; bin_loop < __LENGTH_OF_INT ; bin_loop++)
  {
    //先调整bcd麻对应的10进制数的每一位
    mask_bcd = 0x0000000f ; //column掩码
    shift_bits = 0 ;        //针对每一个column的位移
    for( bcd_loop =0 ; bcd_loop < __LENGTH_OF_INT_BCD ; bcd_loop++ )
    {
      adjust = (ret & mask_bcd) >> shift_bits ; //用于调整每一个column
      if( adjust >= 5 )
      {
        adjust += 3 ;
        adjust &= mask_adjust ;
        ret = (ret & (~(mask_adjust << shift_bits))) | (adjust << shift_bits ) ;
      }
      mask_bcd <<= __COLUMN_SIZE ;
      shift_bits += __COLUMN_SIZE ;
    }
    tmpin = input & mask_h ;
    tmpin >>= __LENGTH_OF_INT - 1 ;
    ret = (ret << 1) | (tmpin & 0x01);
    input <<= 1 ;
  }
  return ret ;  
}