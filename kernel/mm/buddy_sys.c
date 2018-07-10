#include "buddy_sys.h"
#include "dlist.h"
#include "global_type.h"
#include "get_struct_pointor.h"
#include "string.h"
//debug
//#include "kputx.h"
//enddebug

/*
 * 用这个魔数以及链表节点的头指针来表示这个叶帧块没有分配出去
 * 虽然这个方法有缺陷,但省去了bitmap的操作
 */
#define __BUDDY_SYS_MAGIC 0x19841215U

// 向上取整页数
#define up_align( addr ) \
  ( ((addr) & 0x00000fff) ? \
  ( ((addr) & 0xfffff000) + 0x1000) : \
    (addr) )

// 向下取整页数
#define down_align( addr ) \
  ( (addr) & 0xfffff000 )

// 判断是否可以合并,能合并返回TRUE,不能合并返回FALSE
#define can_union( addr, order ) \
  ( (addr) % (((1 << (order)) * __PAGE_SIZE) << 1) ? FALSE : TRUE )
//           (      b         *     2 ^ 12   *  2)
// 找到buddy的首地址
#define find_buddy( addr, order ) \
  ( (addr) + (__PAGE_SIZE << (order)) )
// 判断找到的buddy是否没用被使用,满足三个条件才算未使用
// 1. 存在魔数
// 2. 阶相等
// 3. 头节点正确
#define unused_buddy( addr, order ) \
  ( ((buddy_mgr_node_t *)(addr))->magic == __BUDDY_SYS_MAGIC && \
    ((buddy_mgr_node_t *)(addr))->order == order && \
    ((buddy_mgr_node_t *)(addr))->dlnode.dlhead == &(free_zone.buddy_array[order]) )

typedef struct s_buddy_mgr_node
{
  uint32_t magic ;               // 存储魔数
  uint32_t order ;               // 这一块页的阶
  dlist_node_t dlnode ;          // 用于和 buddy_array 中的链表头组成双现链表
} buddy_mgr_node_t ;

typedef struct s_free_page_array
{
  dlist_head_t buddy_array[__BUDDY_SYS_MAX_ORDER+1] ;   //最大支持1024个页面的分配,即 4M
  //uint32_t free_pages ;
} free_page_array_t ;


static free_page_array_t free_zone ;


// 内存由调用者管理
static inline bool_t init_buddy_mgr_node( buddy_mgr_node_t *bmnode, uint32_t order ) ;
// 合并伙伴关系
static inline void union_buddy( uint32_t phy_addr, uint32_t order ) ;
// 将页面数量转化为order,切保证满足页面申请的要求
static inline uint32_t pgcount_to_order( uint32_t pg_count ) ;


// 初始化伙伴系统
bool_t 
init_buddy_system( uint32_t begin_phy_addr, uint32_t total_mem )
{
  int loop, loop_order ;
  uint32_t align_begin, align_end, pg_addr ;
  dlist_node_t *tmpdnode ;
  buddy_mgr_node_t *tmp_bmnode ;

  align_begin = up_align( begin_phy_addr ) ;
  align_end = down_align( total_mem ) ;

  //debug
  //kputstr( "\n begin addr: " );
  //kput_uint32_hex( align_begin ) ;
  //kputstr( "\n end addr: " );
  //kput_uint32_hex( align_end ) ;
  //kputchar( '\n' ) ;
  //return FALSE ;
  //enddebug

  //debug
  //kputstr( "\ndlist_head_t size is : ") ;
  //kput_uint32_hex( sizeof(dlist_head_t) ) ;
  //kputchar( '\n' ) ;
  //kputstr( "buddy_mgr_node_t size is : ") ;
  //kput_uint32_hex( sizeof(buddy_mgr_node_t) ) ;
  //kputchar( '\n' ) ;
  //enddebug


  //说明没用多余的可用内存了
  if( align_begin > align_end )
    return FALSE ;
  //初始化 free_page_array_t free_zone 数据结构
  //free_zone.free_pages = (align_end - align_begin) / __PAGE_SIZE ;
  for( loop = 0 ; loop <= __BUDDY_SYS_MAX_ORDER ; loop ++ )
    if( init_dlist( &(free_zone.buddy_array[loop] ) ) == FALSE )
      return FALSE ;

  //debug
  //kputstr( "\nfree_zone initail done\n" ) ;
  //enddebug

  //把所有的内存都放到 order = 0 的链表中,然后调用 union_buddy 合并伙伴
  for( pg_addr = align_begin ; pg_addr < align_end ; pg_addr += __PAGE_SIZE )
  {
    tmp_bmnode = (buddy_mgr_node_t *)pg_addr ;
    if( init_buddy_mgr_node( tmp_bmnode, 0 ) )
      // 从链表尾插入,这样链表就是从低地址到高地址排列
      add_dlnode( free_zone.buddy_array[0].tail.prev , &(tmp_bmnode->dlnode) ) ;
  }

  //debug
  //kputstr( "\nbuddy_array begin addr: " ) ;
  //kput_uint32_hex( (uint32_t )free_zone.buddy_array ) ;
  //kputstr( "\n0 order list insert done\n" ) ;
  //kputchar( '\n' ) ;
  //enddebug

  for( loop_order = 0 ; loop_order < __BUDDY_SYS_MAX_ORDER ; loop_order++ )
  {
    loop = free_zone.buddy_array[loop_order].ncnt ;
    while( loop && ( tmpdnode = del_dlnode( free_zone.buddy_array[loop_order].head.next ) ) != NULL )
    {
      loop -- ;
      tmp_bmnode = get_struct_pointor( buddy_mgr_node_t, dlnode, tmpdnode ) ;
      //debug
      //kput_uint32_hex( (uint32_t)tmpdnode ) ;
      //kput_uint32_hex( (uint32_t)tmp_bmnode ) ;
      //enddebug

      union_buddy( (uint32_t )tmp_bmnode, loop_order ) ;
    }
  }

  return TRUE ;
}




// 申请页面.成功,返回物理地址的整数值;失败返回0
// 申请页面的时候,最大可以一次性申请1024个页面
// 由于使用了伙伴系统,所以页面都是2的幂对齐的
// 分配的页面数量会满足需求,但如果请求的页面不是2的幂的数量,
// 则会多分配一些页面,有点浪费哦
uint32_t 
alloc_pages( uint32_t pg_count )
{
  uint32_t order, loop_order ;
  dlist_node_t *dnode ;
  buddy_mgr_node_t *bmnode = 0, *splite_bmnode ;

  if( pg_count == 0 )
    goto ret_val ;

  order = pgcount_to_order( pg_count ) ;
  if( order > __BUDDY_SYS_MAX_ORDER )
    goto ret_val ;
  //debug
  //kputstr( "pg_count :" ) ;
  //kput_uint32_dec( pg_count ) ;
  //kputchar( '\n' ) ;
  //kputstr( "order :" ) ;
  //kput_uint32_dec( order ) ;
  //kputchar( '\n' ) ;
  //return 0 ;
  //enddebug
  if( free_zone.buddy_array[order].ncnt != 0 )
  {
    dnode = del_dlnode( free_zone.buddy_array[order].head.next ) ;
    bmnode = get_struct_pointor( buddy_mgr_node_t, dlnode, dnode ) ;
    goto ret_val ;
  }
  else
  {
    for( loop_order = order + 1 ; loop_order <= __BUDDY_SYS_MAX_ORDER ; loop_order++ )
    {
      if( free_zone.buddy_array[loop_order].ncnt != 0 )
      {
        //把有空闲内存的节点从链表中删除
        dnode = del_dlnode( free_zone.buddy_array[loop_order].head.next ) ;
        bmnode = get_struct_pointor( buddy_mgr_node_t, dlnode, dnode ) ;
        //把节点从原链表中删除,并且分裂成两个节点
        for( ; loop_order > order ; loop_order-- )
        {
          bmnode->order-- ;
          splite_bmnode = (buddy_mgr_node_t *)find_buddy((uint32_t )bmnode, bmnode->order ) ;
          init_buddy_mgr_node( splite_bmnode, bmnode->order ) ;
          //在链表头插入分裂出来的节点
          add_dlnode( &(free_zone.buddy_array[bmnode->order].head), &(splite_bmnode->dlnode) ) ;
        }
        goto ret_val ;
      }
    }
  }
  ret_val :
    if( bmnode != 0 )
      memset( (void *)bmnode, 0, (1 << order) * __PAGE_SIZE ) ;
    return (uint32_t )bmnode ;
}
// 释放页面.
void 
free_pages( uint32_t pg_phy_addr, uint32_t pg_count )
{
  uint32_t order ;
  if( pg_count == 0 )
    return ;
  order = pgcount_to_order( pg_count ) ;
  if( order > __BUDDY_SYS_MAX_ORDER )
    return ;
  //debug
  //kputstr( "pg_count :" ) ;
  //kput_uint32_dec( pg_count ) ;
  //kputchar( '\n' ) ;
  //kputstr( "order :" ) ;
  //kput_uint32_dec( order ) ;
  //kputchar( '\n' ) ;
  //return 0 ;
  //enddebug
  union_buddy( pg_phy_addr, order ) ;
}



static inline bool_t 
init_buddy_mgr_node( buddy_mgr_node_t *bmnode, uint32_t order )
{
  bmnode->magic = __BUDDY_SYS_MAGIC ;
  bmnode->order = order ;
  return init_dlnode( &(bmnode->dlnode) ) ;
}


static inline void 
union_buddy( uint32_t phy_addr, uint32_t order )
{
  buddy_mgr_node_t *bmnode, *buddy_bmnode ;
  uint32_t buddy_addr ;

  // 哈哈 伙伴系统出错了,超出了最大的 order
  if( order > __BUDDY_SYS_MAX_ORDER )
    return ;

  if( order == __BUDDY_SYS_MAX_ORDER )
    goto insert_to_list ;

  //debug
  //kputstr( "\ncan union? \n" ) ;
  //enddebug
  if( can_union( phy_addr, order ) )
  {
    buddy_addr = find_buddy( phy_addr, order ) ;
    //debug
    //kputstr( "\ncan union\nbuddy_addr = " ) ;
    //kput_uint32_hex( buddy_addr ) ;
    //kputchar('\n') ;
    //enddebug
    if( unused_buddy( buddy_addr, order ) )
    {
      // 合并伙伴关系
      buddy_bmnode = (buddy_mgr_node_t *)buddy_addr ;
      // 从相应order链表中删除伙伴
      del_dlnode( &(buddy_bmnode->dlnode) ) ;
      union_buddy( phy_addr, order + 1 ) ;
    }
    else
      goto insert_to_list ;
  }
  else
  {
    insert_to_list :
      bmnode = (buddy_mgr_node_t *)phy_addr ;
      init_buddy_mgr_node( bmnode, order ) ;
      //在链表末尾添加新的节点
      add_dlnode( free_zone.buddy_array[order].tail.prev, &(bmnode->dlnode) ) ;
  }

  return ;
}

static inline uint32_t
pgcount_to_order( uint32_t pg_count )
{
  uint32_t order ;
  uint32_t mask = 0xffffffff ;
  asm volatile ( "bsr %k1,%k0" : "=a"(order) : "b"(pg_count) : "cc", "memory" ) ;
  if( order != 0 )
  {
    mask >>= ( sizeof(uint32_t) * 8 - order ) ;
    if( (pg_count & mask) != 0 )
      order++ ;
  }

  return order ;
}






