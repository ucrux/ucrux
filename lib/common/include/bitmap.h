#ifndef __BITMAP_H
#define __BITMAP_H
#include "global_type.h"



#define __BITMAP_UNIT       0x08U
#define __BITMAP_MASK       0x01U



#define __BITMAP32_LEN        16U
#define __BITMAP64_LEN        8U
#define __BITMAP128_LEN       4U
#define __BITMAP256_LEN       2U
#define __BITMAP512_LEN       1U
#define __BITMAP1024_LEN      1U
#define __BITMAP2048_LEN      1U
#define __BITMAP_HD_BLOCK_LEN 4096U
#define __BITMAP_HD_INODE_LEN 4096U


#define get_bmap_bits_addr( bmap ) \
          ( (uint8_t *)( (char *)(bmap) + sizeof(uint32_t)) )



typedef enum e_bitmap_type
{
  T_BITMAP_32 ,
  T_BITMAP_64 ,
  T_BITMAP_128 ,
  T_BITMAP_256 ,
  T_BITMAP_512 ,
  T_BITMAP_1024 ,
  T_BITMAP_2048 ,
  T_BITMAP_HD_BLOCK,
  T_BITMAP_HD_INODE
} bitmap_type_t ;




typedef struct s_bitmap
{
  uint32_t length ;
} bitmap_t ;


typedef struct s_bitmap32
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP32_LEN] ;
} bitmap32_t ;

typedef struct s_bitmap64
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP64_LEN] ;
} bitmap64_t ;

typedef struct s_bitmap128
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP128_LEN] ;
} bitmap128_t ;

typedef struct s_bitmap256
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP256_LEN] ;
} bitmap256_t ;

typedef struct s_bitmap512
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP512_LEN] ;
} bitmap512_t ;

typedef struct s_bitmap1024
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP1024_LEN] ;
} bitmap1024_t ;

typedef struct s_bitmap2048
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP2048_LEN] ;
} bitmap2048_t ;

typedef struct s_bitmap_hd_block
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP2048_LEN] ;
} bitmap_hd_block_t ;

typedef struct s_bitmap_hd_inode
{ 
  uint32_t length ;
  uint8_t bits[__BITMAP2048_LEN] ;
} bitmap_hd_inode_t ;

/*将位图中的指定位置1*/
/*越界返回FALSE, bmap是个指针*/
#define bitmap_set( bmap, idx ) \
  ( \
    ( (((uint32_t )(idx)) >= ((bmap)->length * __BITMAP_UNIT)) || ((int )(idx)) < 0 ) ? FALSE : \
    ((get_bmap_bits_addr( bmap )[((uint32_t )(idx))/__BITMAP_UNIT] |= \
     (__BITMAP_MASK << (((uint32_t )(idx)) % __BITMAP_UNIT))), TRUE) \
   )

/*将位图中的指定位清0*/
/*越界返回FALSE*/
#define bitmap_clear( bmap, idx ) \
  ( \
    ( (((uint32_t )(idx)) >= ((bmap)->length * __BITMAP_UNIT)) || ((int )(idx)) < 0 ) ? FALSE : \
    ((get_bmap_bits_addr( bmap )[((uint32_t )(idx))/__BITMAP_UNIT] &= \
      ~(__BITMAP_MASK << (((uint32_t )(idx)) % __BITMAP_UNIT))), TRUE) \
  )

/*测位图中某一位的值*/
/*越界返回ERROR*/
#define bitmap_test( bmap, idx ) \
  ( \
    ( (((uint32_t )(idx)) >= ((bmap)->length * __BITMAP_UNIT)) || ((int )(idx)) < 0 ) ? ERROR : \
    ((get_bmap_bits_addr( bmap )[((uint32_t )(idx))/__BITMAP_UNIT] & \
      (__BITMAP_MASK << (((uint32_t )(idx)) % __BITMAP_UNIT))) ? 1 : 0 ) \
  )

/*初始化位图,此处bmap由调用管理,bmap->bits内存由调用者管理*/
bool_t bmap_init( bitmap_t *bmap, bitmap_type_t bmap_type ) ;

/*扫描位图,找到1个空闲位,返回其在bitmap中的索引*/
int bmap_scan( bitmap_t *bmap ) ;


#endif
