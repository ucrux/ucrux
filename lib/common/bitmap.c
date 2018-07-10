#include "bitmap.h"
#include "string.h"



/*初始化位图,此处bmap由调用管理,*/
bool_t
bmap_init( bitmap_t *bmap, bitmap_type_t bmap_type )
{
  if( bmap == NULL )
    return FALSE ;

  switch (bmap_type)
  {
    case T_BITMAP_32 :
      bmap->length = __BITMAP32_LEN ;
      break ;
    case T_BITMAP_64 :
      bmap->length = __BITMAP64_LEN ;
      break ;
    case T_BITMAP_128 :
      bmap->length = __BITMAP128_LEN ;
      break ;
    case T_BITMAP_256 :
      bmap->length = __BITMAP256_LEN ;
      break ;
    case T_BITMAP_512 :
    case T_BITMAP_1024 :
    case T_BITMAP_2048 :
      bmap->length = __BITMAP512_LEN ;
      break ;
    default :
      return FALSE ;
  }

  memset( (void *)get_bmap_bits_addr( bmap ), 0, bmap->length ) ;

  // 特殊处理1024 和 2048的 bmap
  if( bmap_type == T_BITMAP_1024 )
    get_bmap_bits_addr( bmap )[0] |= 0xf0U ;

  if( bmap_type == T_BITMAP_2048 )
    get_bmap_bits_addr( bmap )[0] |= 0xfcU ;

  return TRUE ;
}


/*扫描位图,找到1个空闲位,返回其在bitmap中的索引*/
int
bmap_scan( bitmap_t *bmap )
{
  if( bmap == NULL )
    return -1 ;

  uint32_t idx ;

  for( idx = 0 ; idx < bmap->length * __BITMAP_UNIT ; idx++ )
    if( bitmap_test( bmap, idx ) == 0 )
      break ;

  if( ((uint32_t )idx) >= bmap->length * __BITMAP_UNIT )
    idx = -1 ;

  return (int )idx ;
}
