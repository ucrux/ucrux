#include "mm.h"
#include "get_struct_pointor.h"
#include "page.h"


// 初始化mm_blob_t结构,mblob的内存由调用者管理
bool_t
init_mm_blob( mm_blob_t *mblob, \
              hash_fun_t hash, \
              hash_getkey_fun_t getkey, \
              hash_keycmp_fun_t cmp  )
{
  int btype ;

  if( mblob == NULL )
    return FALSE ;

  for( btype = 0 ; btype < BLOB_TYPE_CNT ; btype++ )
    if( init_dlist( &(mblob->barray[btype]) ) == FALSE )
      return FALSE ;

  if( init_hashtb( &(mblob->hashtb), hash, getkey, cmp ) == FALSE )
    return FALSE ;

  return TRUE ;
}

// 初始化 mm_blobxx_t 节点, blobxx节点由点用者管理
bool_t
init_mm_blobxx( mm_blobxx_t* blobxx, mm_blob_type_t btype, uint32_t vaddr )
{
  blobxx->vaddr = vaddr ;
  bitmap_type_t bmap_type ;
  switch (btype)
  {
    case BLOB_32 :
      blobxx->remain_blob_cnt = __BITMAP_UNIT * __BITMAP32_LEN ;
      bmap_type = T_BITMAP_32 ;
      break ;
    case BLOB_64 :
      blobxx->remain_blob_cnt = __BITMAP_UNIT * __BITMAP64_LEN ;
      bmap_type = T_BITMAP_64 ;
      break ;
    case BLOB_128 :
      blobxx->remain_blob_cnt = __BITMAP_UNIT * __BITMAP128_LEN ;
      bmap_type = T_BITMAP_128 ;
      break ;
    case BLOB_256 :
      blobxx->remain_blob_cnt = __BITMAP_UNIT * __BITMAP256_LEN ;
      bmap_type = T_BITMAP_256 ;
      break ;
    case BLOB_512 :
      blobxx->remain_blob_cnt = __BITMAP_UNIT * __BITMAP512_LEN ;
      bmap_type = T_BITMAP_512 ;
      break ;
    case BLOB_1024 :
      blobxx->remain_blob_cnt = 4 ;
      bmap_type = T_BITMAP_1024 ;
      break ;
    case BLOB_2048 :
      blobxx->remain_blob_cnt = 2 ;
      bmap_type = T_BITMAP_2048 ;
      break ;
    default :
      return FALSE ;
  }

  blobxx->all_blob_cnt = blobxx->remain_blob_cnt ;
  blobxx->btype = btype ;

  if( init_dlnode( &(blobxx->lnode) ) == FALSE || \
      init_dlnode( &(blobxx->freenode) ) == FALSE || \
      bmap_init( &(blobxx->bmap), bmap_type ) == FALSE )
    return FALSE ;

  return TRUE ;
}

// 向 mm_blob_t 插入 mm_blobxx_t节点
// 向 barray[btype] 加入一个节点(barray[btype]之前必须 == NULL)
// 将新节点插入到 hashtb
bool_t
insert_mm_blobxx( mm_blob_t *mblob, mm_blobxx_t *blobxx, mm_blob_type_t btype )
{
  if( mblob == NULL || blobxx == NULL )
    return FALSE ;
  if( btype < BLOB_32 || btype > BLOB_2048 )
    return FALSE ;

  if( add_dlnode( &(mblob->barray[btype].head), &(blobxx->freenode) ) == FALSE )
    return FALSE ;

  if( insert_hashtb( &(mblob->hashtb), &(blobxx->lnode) ) == FALSE )
  {
    del_dlnode( &(blobxx->freenode) ) ;
    return FALSE ;
  }

  return TRUE ;
}

// 从 mm_blob_t 中删除 mm_blobxx_t 节点
mm_blobxx_t *
delete_mm_blobxx( mm_blob_t *mblob, uint32_t vaddr )
{
  dlist_node_t *dlnode ;
  mm_blobxx_t *blobxx ;

  if( mblob == NULL || vaddr == 0 )
    return NULL ;

  dlnode = srch_hashtb( &(mblob->hashtb), (void *)(vaddr & __PG_MASK) ) ;
  if( dlnode == NULL )
    return NULL ;
  blobxx = get_struct_pointor( mm_blobxx_t, lnode, dlnode ) ;

  del_hashtb( &(mblob->hashtb), (void *)(vaddr & __PG_MASK) ) ;
  del_dlnode( &(blobxx->freenode) ) ;


  return blobxx ;
}

// 分配一个单位的blob内存,成功返回这块内存的虚拟地址
uint32_t
alloc_blob( mm_blob_t *mblob, mm_blob_type_t btype )
{
  dlist_node_t *dlnode ;
  mm_blobxx_t *blobxx ;
  int idx ;
  uint32_t unit ;

  switch (btype)
  {
    case BLOB_32 :
      unit = 32 ;
      break ;
    case BLOB_64 :
      unit = 64 ;
      break ;
    case BLOB_128 :
      unit = 128 ;
      break ;
    case BLOB_256 :
      unit = 256 ;
      break ;
    case BLOB_512 :
      unit = 512 ;
      break ;
    case BLOB_1024 :
      unit = 1024 ;
      break ;
    case BLOB_2048 :
      unit = 2048 ;
      break ;
    default :
      return 0 ;
  }

  // 空闲列表中不为空
  if( mblob->barray[btype].ncnt > 0 )
  {
    dlnode = mblob->barray[btype].head.next ;
    blobxx = get_struct_pointor( mm_blobxx_t, freenode, dlnode ) ;
    idx = bmap_scan( &(blobxx->bmap) ) ;
    if( idx == -1 )
      return 0 ;
    bitmap_set( &(blobxx->bmap), idx ) ;
    blobxx->remain_blob_cnt-- ;
    if( blobxx->remain_blob_cnt == 0 )
      del_dlnode( dlnode ) ;
    return blobxx->vaddr + unit * (uint32_t)idx ;
  }

  return 0 ;
}

// 释放一个单位的blob内存
// 如果这个blobxx已经全空,就返回这个blob的线性地址
// 否则就返回 0
// 如果释放的内存不在blob分配里面或者出错,返回 -1
int 
free_blob( mm_blob_t *mblob, uint32_t vaddr )
{
  dlist_node_t *dlnode ;
  mm_blobxx_t *blobxx ;
  int idx ;
  uint32_t unit ;

  if( mblob == NULL )
    return -1 ;

  dlnode = srch_hashtb( &(mblob->hashtb), (void *)(vaddr & __PG_MASK) ) ;

  if( dlnode == NULL )
    return -1 ;
  blobxx = get_struct_pointor( mm_blobxx_t, lnode, dlnode ) ;
  
  switch (blobxx->btype)
  {
    case BLOB_32 :
      unit = 32 ;
      break ;
    case BLOB_64 :
      unit = 64 ;
      break ;
    case BLOB_128 :
      unit = 128 ;
      break ;
    case BLOB_256 :
      unit = 256 ;
      break ;
    case BLOB_512 :
      unit = 512 ;
      break ;
    case BLOB_1024 :
      unit = 1024 ;
      break ;
    case BLOB_2048 :
      unit = 2048 ;
      break ;
    default :
      return -1 ;
  }

  idx = (vaddr - blobxx->vaddr) / unit ;

  if( bitmap_test( &(blobxx->bmap), idx ) == 1 )
  {
    bitmap_clear( &(blobxx->bmap), idx ) ;
    blobxx->remain_blob_cnt++ ; 
  }


  if( blobxx->remain_blob_cnt > 0 )
    add_dlnode( &(mblob->barray[blobxx->btype].head), &(blobxx->freenode) ) ;
  
  if( blobxx->remain_blob_cnt == blobxx->all_blob_cnt )
  {
    del_hashtb( &(mblob->hashtb), (void *)(vaddr & __PG_MASK) ) ;
    del_dlnode( &(blobxx->freenode) ) ;
    return (int )blobxx ;
  }

  return 0 ;
}
