#include "kernel_mm.h"
#include "hash.h"
#include "dlist.h"
#include "buddy_sys.h"
#include "kernel.h"
#include "get_struct_pointor.h"
#include "global_type.h"
#include "page.h"
#include "proc.h"


extern void change_pdt( uint32_t *pdt ) ;

#define __INIT_BLOCK_CNT   63  //mm_blob32_t 是所有 mm_blobxx_t 结构中最大的
                               //大小为 60 + bool_t used = 64,
                               //所以一个页面最多放下64个block_blobxx_t
                               //剩下的空间放一些其他的管理数据

// 这个数据结构用来保存mm_blobxx_t结构
typedef struct s_block_blobxx
{
  bool_t used ;       // 初始化为 FASLE,表示没有被使用
  mm_blob32_t block ; // mm_blob32_t 是所有 mm_blobxx_t 结构中最大的
} block_blobxx_t ;

// 这个数据结构使用分配的页面保存
// 如果一个blobxx_pg_manage_t结构满了,就将其放入链表尾
// 有空闲空间,就将其放到链表头
typedef struct s_blobxx_pg_manage
{
  block_blobxx_t block_array[__INIT_BLOCK_CNT] ;
  uint32_t all_cnt ;    // 初始化为 __INIT_BLOCK_CNT
  uint32_t remain_cnt ; // 初始化为 all_cnt
  dlist_node_t lnode ;
} blobxx_pg_manage_t ;

// 这个数据结构用来保存 vaddr_node_t 结构
typedef struct s_block_vaddrnode
{
  bool_t used ;
  vaddr_node_t block ;
} block_vaddrnode_t ;
// 这个数据结构使用分配的页面保存
// 如果一个vaddrnode_pg_manage_t结构满了,就将其放入链表尾
// 有空闲空间,就将其放到链表头
typedef struct s_vaddrnode_pg_manage
{
  block_vaddrnode_t block_array[__INIT_BLOCK_CNT] ;
  uint32_t all_cnt ;    // 初始化为 __INIT_BLOCK_CNT
  uint32_t remain_cnt ; // 初始化为 all_cnt
  dlist_node_t lnode ;
} vaddrnode_pg_manage_t ;


// 创建一个blobxx_pg_manage_t结构
// 初始化并将其插入到blobxx_alloc的头节点后
static bool_t mk_blobxx_pg_manage( pcb_t *pcb ) ;
// 分配一个mm_blobxx_t的存储空间
static mm_blobxx_t *mmblobxx_alloc( pcb_t *pcb ) ;
// 释放一个mm_blobxx_t的存储空间
static void mmblobxx_free( pcb_t *pcb, mm_blobxx_t *ptr ) ;

// 创建一个vaddrnode_pg_manage_t结构
// 初始化并将其插入到vaddrnode_alloc的头节点后
static bool_t mk_vaddrnode_pg_manage( pcb_t *pcb ) ;
// 分配一个vaddr_node_t的存储空间
static vaddr_node_t *vaddrnode_alloc( pcb_t *pcb ) ;
// 释放一个vaddr_node_t的存储空间
static void vaddrnode_free( pcb_t *pcb, vaddr_node_t *ptr ) ;


// 内核内存分配函数
/*
 * 注意事项
 * 首先要确定内存真正的分配大小
 * kmalloc 和 kfree 分为 给内核空间分配内存和给用户空间分配内存
 * 需要进行区分
 */
void *
kmalloc( uint32_t size, pcb_t *pcb )
{
  pcb_t *current = NULL, *kernel_pcb = (pcb_t *)__KERNEL_PCB ;
  mm_blob_type_t btype = BLOB_TYPE_CNT ;
  uint32_t real_size = 0 ;
  uint32_t pg_cnt = 0 ;
  uint32_t addr = 0, blob_addr, vaddr ;
  mm_blobxx_t *mm_blobxx = NULL ;
  vaddr_node_t *vaddrnode ;
  vaddr_t key_vaddr ;
  uint32_t loop, loop_pg ;

  if( pcb == NULL )
    return NULL ;

  current = pcb ;

  if( size <= 32 )
  {
    real_size = 32 ;
    btype = BLOB_32 ;
  }
  else if( size <= 64 )
  {
    real_size = 64 ;
    btype = BLOB_64 ;
  }
  else if( size <= 128 )
  {
    real_size = 128 ;
    btype = BLOB_128 ;
  }
  else if( size <= 256 )
  {
    real_size = 256 ;
    btype = BLOB_256 ;
  }
  else if( size <= 512 )
  {
    real_size = 512 ;
    btype = BLOB_512 ;
  }
  else if( size <= 1024 )
  {
    real_size = 1024 ;
    btype = BLOB_1024 ;
  }
  else if( size <= 2048 )
  {
    real_size = 2048 ;
    btype = BLOB_2048 ;
  }
  else
  {
    pg_cnt = size / __PAGE_SIZE  ;
    if( (size % __PAGE_SIZE) != 0 )
      pg_cnt++ ;
  }

  if( real_size != 0 ) // blob 分配
  {
    if( current->mmblob->barray[btype].ncnt == 0 )  // blob分配上没有管理节点
    {                                              // 即此种类型blob没有内存池
      // 因为没有可用的blob池,所以一定要分配一页内存到内核的虚拟地址空间
      // 分配一页内存用于blob分配的内存池
      blob_addr = alloc_pages( 1 ) ;
      if( blob_addr == 0 )                     //没有可用内存了
        goto retval ;

      // 用户空间分配,如果不是在内核空间分配
      // 那么用户空间页需要分配虚拟地址
      if( current != kernel_pcb )
      {
        // 用户进程使用虚拟地址,分配一个虚拟地址给用户空间
        if( (vaddr = alloc_vaddr( &(current->vaddr_manage), 1 ) ) == 0 )
        {
          // 虚拟地址用完了,直接进行清理
          free_pages( blob_addr, 1 ) ;
          goto retval ;
        }
      }
      else
        // 内核使用平坦模型
        vaddr = blob_addr ;

      // 分配blob分配管理节点
      if( (mm_blobxx = mmblobxx_alloc( current )) == NULL )
      {
        free_pages( blob_addr, 1 ) ;   //blob分配管理节点分配失败,进行清理
        goto retval ;
      }
      // 初始化管理节点并插入到mmblob分配池
      if( init_mm_blobxx( mm_blobxx, btype, vaddr ) == FALSE || \
          insert_mm_blobxx( current->mmblob, mm_blobxx, btype ) == FALSE )
      {
        // 初始化blob管理节点失败,进行清理
        free_pages( blob_addr, 1 ) ;
        mmblobxx_free( current, mm_blobxx ) ;
        goto retval ;
      }

      // 将此页内存加入内核虚拟地址管理
      // 分配虚拟地址管理节点
      if( (vaddrnode = vaddrnode_alloc( current ) ) == NULL )
      {
        // 分配虚拟地址管理失败,进行清理
        free_pages( blob_addr , 1 ) ;
        delete_mm_blobxx( current->mmblob, vaddr ) ;
        mmblobxx_free( current, mm_blobxx ) ;
        goto retval ;
      }
      // 添加虚拟地址到虚拟地址管理池
      if( init_vaddr_node( vaddrnode, vaddr, 1 ) == FALSE || \
          inst_vaddr_node( &(current->vaddr_manage), vaddrnode ) == FALSE )
      {
        // 初始化虚拟内存管理节点,并将其插入到虚拟地址池失败,进行清理
        free_pages( blob_addr , 1 ) ;
        delete_mm_blobxx( current->mmblob, vaddr ) ;
        mmblobxx_free( current, mm_blobxx ) ;
        vaddrnode_free( current, vaddrnode ) ;
        goto retval ;
      }
      // 如果是用户进程,添加虚拟地址映射
      if( current != kernel_pcb )
        if( map_page( current->pdt, vaddr, blob_addr ) == FALSE )
        {
          //映射失败,清理起来
          key_vaddr.vaddr_begin = vaddr ;
          key_vaddr.pg_cnt = 1 ;
          free_pages( blob_addr , 1 ) ;
          delete_mm_blobxx( current->mmblob, vaddr ) ;
          mmblobxx_free( current, mm_blobxx ) ;
          delete_vaddr_node( &(current->vaddr_manage), &key_vaddr ) ;
          vaddrnode_free( current, vaddrnode ) ;
          goto retval ;
        }
    }
    addr = alloc_blob( current->mmblob, btype ) ;
  }
  else                 // 页分配
  {
    blob_addr = alloc_pages( pg_cnt ) ; //分配所需要的内存页
    // 用户空间需要分配虚拟地址
    if( current != kernel_pcb )
    {
      if( (vaddr = alloc_vaddr( &(current->vaddr_manage), pg_cnt ) ) == 0 )
      {
        // 分配虚拟地址失败,进行清理
        free_pages( blob_addr, pg_cnt ) ;
        goto retval ;
      }
    }
    else  // 内核空间使用平坦模型
      vaddr = blob_addr ;

   
    // 分配虚拟地址管理节点
    if( (vaddrnode = vaddrnode_alloc( current ) ) == NULL )
    {
      // 分配虚拟地址管理节点失败,进行清理
      free_pages( blob_addr , pg_cnt ) ;
      goto retval ;
    }
    // 添加虚拟地址到虚拟地址管理池
    if( init_vaddr_node( vaddrnode, vaddr, pg_cnt ) == FALSE || \
        inst_vaddr_node( &(current->vaddr_manage), vaddrnode ) == FALSE )
    {
      // 虚拟地址管理节点分配失败,或者插入虚拟地池失败
      // 进行清理
      free_pages( blob_addr , pg_cnt ) ;
      vaddrnode_free( current, vaddrnode ) ;
      goto retval ;
    }
    // 用户空间需要映射叶帧
    if( current != kernel_pcb )
    {
      for( loop_pg = 0 ; loop_pg < pg_cnt ; loop_pg++ )
      {
        if( map_page( current->pdt, \
                      vaddr + loop_pg * __PAGE_SIZE, \
                      blob_addr + loop_pg * __PAGE_SIZE ) == FALSE )
        {
          // 虚拟地址映射失败,进行清理
          // 清理已有映射
          for( loop = 0 ; loop < loop_pg ; loop++ )
            unmap_page( current->pdt, vaddr + loop * __PAGE_SIZE ) ;
          //清理其他已申请的资源
          key_vaddr.vaddr_begin = vaddr ;
          key_vaddr.pg_cnt = 1 ;
          free_pages( blob_addr , pg_cnt ) ;
          delete_vaddr_node( &(current->vaddr_manage), &key_vaddr ) ;
          vaddrnode_free( current, vaddrnode ) ;
          goto retval ;
        }
      }
    }
    addr = vaddr ;
  }

  retval :
    return (void *)addr ;
}


// 内核内存释放函数
void 
kfree( void *ptr, pcb_t *pcb )
{
  int blob_ret ;
  pcb_t *current = NULL, *kernel_pcb = (pcb_t *)__KERNEL_PCB ;
  mm_blobxx_t *mmblobxx ;
  vaddr_t nodevddr ;
  vaddr_node_t *vnode ;
  uint32_t loop ;

  if( ptr == NULL || pcb == NULL )
    return ;
  
  current = pcb ;

  // 尝试使用 blob 释放, 失败会返回 -1
  blob_ret = free_blob( current->mmblob, (uint32_t )ptr ) ;


  
  // 此地址不在 blob 中, 使用伙伴系统释放
  if( blob_ret == -1 )
  {
    // 判断这个地址是不是页对齐的,如果不是页对齐的,也不是页伙伴系统管理
    if( ( ((uint32_t)ptr) & (~__PG_MASK) ) == 0 ) 
    {
      // 检查是否在 vaddr_manage 的管理中
      nodevddr.vaddr_begin = (uint32_t )ptr ;
      nodevddr.pg_cnt = 0 ;
      // vaddr_t nodevaddr 是虚拟地址池的关键字
      // 此虚拟地址不在虚拟地址池中,直接返回
      if( ( vnode = delete_vaddr_node( &(current->vaddr_manage), &nodevddr ) ) == NULL )
        return ;
      // 通过物理地址释放页面
      free_pages( get_pg_phy_addr( (uint32_t )ptr, (uint32_t )current->pdt ), vnode->vaddr.pg_cnt ) ;
      // 用户空间要解除叶帧映射
      if( current != kernel_pcb )
        for( loop = 0 ; loop < vnode->vaddr.pg_cnt ; loop++ )
          unmap_page( current->pdt, vnode->vaddr.vaddr_begin + loop * __PAGE_SIZE ) ;
      vaddrnode_free( current, vnode ) ;
    }
  }
  // 此地址在 blob 中, 且此 blob 已经不使用了
  else if( blob_ret != 0 )
  {
    mmblobxx = (mm_blobxx_t *)blob_ret ;
    // 归还 blob 内存池中的内存
    free_pages( get_pg_phy_addr( mmblobxx->vaddr, (uint32_t )current->pdt ), 1 ) ;
    // 归还 bloo 管理节点内存,在blob内存池中删除 mmblobxx, 因为free_blob已经将其从链表中删除
    mmblobxx_free( current, mmblobxx ) ;
    // 用户空间需要解除者一页的映射
    if( current != kernel_pcb )
      unmap_page( current->pdt, (uint32_t )ptr & __PG_MASK ) ;
    // 释放虚拟地址管理节点
    nodevddr.vaddr_begin = (uint32_t )ptr & __PG_MASK ;
    nodevddr.pg_cnt = 0 ;
    vaddrnode_free( current, delete_vaddr_node( &(current->vaddr_manage), &nodevddr ) ) ;
  }
}

// 系统调用
void *
sys_malloc( uint32_t size )
{
  pcb_t *pcb ;
  void *ptr ;

  // 获取当前进程的PCB的地址
  get_current_pcb( pcb ) ;
  ptr = kmalloc( size, pcb ) ;

  /*
   * 这是为了刷新TLB,这两天程序出错都是因为TLB没有刷新
   * 这个方法使TLB失效,似乎没有作用
   * asm volatile ("invlpg %0" : :"m" (vaddr) : "memory") ;
   */

  change_pdt( pcb->pdt ) ;

  return ptr ;
}


void
sys_free( void *ptr )
{
  pcb_t *pcb ;
  // 获取当前进程的PCB的地址
  get_current_pcb( pcb ) ;


  kfree( ptr, pcb ) ;

  change_pdt( pcb->pdt ) ;
}


// 局部函数
// 这几个函数所使用的内存和用户空间没有关系,所以不需要映射页表
// 且以下函数分配或释放的内存不接受任何管理
// 释放一个mm_blobxx_t的存储空间
static void 
mmblobxx_free( pcb_t *pcb, mm_blobxx_t *ptr )
{
  if( ptr == NULL || pcb == NULL )
    return ;

  blobxx_pg_manage_t *blob_pg_mm_1st ;
  dlist_node_t *dlnode = NULL ;
  uint32_t blob_pg_mm_addr = (uint32_t )ptr & __PG_MASK ;
  blobxx_pg_manage_t *blob_pg_mm = (blobxx_pg_manage_t *)blob_pg_mm_addr ;
  block_blobxx_t *block_blobxx = get_struct_pointor( block_blobxx_t, block, ptr ) ;


  block_blobxx->used = FALSE ;  //清除为没使用
  blob_pg_mm->remain_cnt++ ;

  // 这个管理节点已经没有使用了,可以归还给伙伴系统了
  if( blob_pg_mm->remain_cnt == blob_pg_mm->all_cnt )
  {
    del_dlnode( &(blob_pg_mm->lnode) ) ;
    free_pages( blob_pg_mm_addr, 1 ) ;
  }
  else
  {
    // 判断是否要将节点提到表头
    // 如果本链表中的第一个节点也有空闲块就不将此节点提到表头
    // 这么做可以提高回收效率
    dlnode = pcb->blobxx_alloc.head.next ; // 取得第一个节点
    // 获取blobxx_pg_manage_t结构体指针
    blob_pg_mm_1st = get_struct_pointor( blobxx_pg_manage_t, lnode, dlnode ) ;
    if( blob_pg_mm_1st->remain_cnt == 0 ) // 第一个节点是一个已使用完的节点
      add_dlnode( &(pcb->blobxx_alloc.head), del_dlnode( &(blob_pg_mm->lnode) ) ) ;
  }
}

// 分配一个mm_blobxx_t的存储空间
static mm_blobxx_t *
mmblobxx_alloc( pcb_t *pcb )
{
  uint32_t loop ;
  blobxx_pg_manage_t *blob_pg_mm ;
  dlist_node_t *dlnode = NULL ;

  if( pcb == NULL )
    return NULL ;

  // 管理节点内存池链表中存在管理节点内存分配节点
  if( pcb->blobxx_alloc.ncnt != 0 )   // 有节点在链表中
  {
    dlnode = pcb->blobxx_alloc.head.next ; // 取得第一个节点
    // 获取blobxx_pg_manage_t结构体指针
    blob_pg_mm = get_struct_pointor( blobxx_pg_manage_t, lnode, dlnode ) ; 
    // 管理节点中还有空闲内存
    if( blob_pg_mm->remain_cnt != 0 )
      goto blob_pg_mm_done ;   //直接去分配内存
  }
  // 管理节点中都没有可用内存了,分配新的管理节点
  if( mk_blobxx_pg_manage( pcb ) == FALSE )
    return NULL ;  //没有多余的内存了

  blob_pg_mm_done :     //可以分配块了
    dlnode = pcb->blobxx_alloc.head.next ;
    blob_pg_mm = get_struct_pointor( blobxx_pg_manage_t, lnode, dlnode ) ;

    // 查找空闲块
    for( loop = 0; loop < __INIT_BLOCK_CNT ; loop++ )
    {
      if( blob_pg_mm->block_array[loop].used == FALSE ) //找到空闲块
      {
        blob_pg_mm->block_array[loop].used = TRUE ;
        blob_pg_mm->remain_cnt-- ;
        //判断是否要将这个节点移动到链表尾
        if( blob_pg_mm->remain_cnt == 0 ) //已经没有空闲块了
          // 在链表尾插入
          add_dlnode( pcb->blobxx_alloc.tail.prev, del_dlnode( &(blob_pg_mm->lnode) ) ) ;

        return (mm_blobxx_t *)&(blob_pg_mm->block_array[loop].block) ;
      }
    }

  return NULL ;
}


// 创建一个blobxx_pg_manage_t结构
// 初始化并将其插入到current->blobxx_alloc.head的头节点后
static bool_t 
mk_blobxx_pg_manage( pcb_t *pcb )
{
  blobxx_pg_manage_t *blob_pg_mm ;
  uint32_t loop ;

  if( pcb == NULL )
    return FALSE ;

  uint32_t pg_addr = alloc_pages( 1 ) ;
  if( pg_addr == 0 )  // 没有可用内存了
    return FALSE ;
  
  blob_pg_mm = (blobxx_pg_manage_t *)pg_addr ;
  blob_pg_mm->all_cnt = __INIT_BLOCK_CNT ;
  blob_pg_mm->remain_cnt = __INIT_BLOCK_CNT ;
  if( init_dlnode( &(blob_pg_mm->lnode) ) == FALSE )
  {
    free_pages( pg_addr, 1 ) ;
    return FALSE ;
  }

  for( loop = 0 ; loop < __INIT_BLOCK_CNT ; loop++ )
    blob_pg_mm->block_array[loop].used = FALSE ;

  if( add_dlnode( &(pcb->blobxx_alloc.head), &(blob_pg_mm->lnode) ) == FALSE )
  {
    free_pages( pg_addr, 1 ) ;
    return FALSE ;
  }
  return TRUE ;
}



// 释放一个vaddr_node_t的存储空间
static void 
vaddrnode_free( pcb_t *pcb, vaddr_node_t *ptr )
{
  if( ptr == NULL || pcb == NULL )
    return ;

  vaddrnode_pg_manage_t *vnode_pg_1st ;
  dlist_node_t *dlnode = NULL ;
  uint32_t vnode_pg_addr = (uint32_t )ptr & __PG_MASK ;
  vaddrnode_pg_manage_t *vnode_pg  = (vaddrnode_pg_manage_t *)vnode_pg_addr ;
  block_vaddrnode_t *block_vnode = get_struct_pointor( block_vaddrnode_t, block, ptr ) ;

  block_vnode->used = FALSE ;  //清除为未使用
  vnode_pg->remain_cnt++ ;

  // 这个管理节点已经没有使用了,可以归还给伙伴系统了
  if( vnode_pg->remain_cnt == vnode_pg->all_cnt )
  {
    del_dlnode( &(vnode_pg->lnode) ) ;
    free_pages( vnode_pg_addr, 1 ) ;
  }
  else
  {
    // 判断是否要将节点提到表头
    // 如果本链表中的第一个节点也有空闲块就不将此节点提到表头
    // 这么做可以提高回收效率
    dlnode = pcb->vaddrnode_alloc.head.next ; // 取得第一个节点
    // 获取blobxx_pg_manage_t结构体指针
    vnode_pg_1st = get_struct_pointor( vaddrnode_pg_manage_t, lnode, dlnode ) ;
    if( vnode_pg_1st->remain_cnt == 0 ) // 第一个节点是一个已使用完的节点
      add_dlnode( &(pcb->vaddrnode_alloc.head), del_dlnode( &(vnode_pg->lnode) ) ) ;
  }
}

// 分配一个vaddr_node_t的存储空间
static vaddr_node_t *
vaddrnode_alloc( pcb_t *pcb )
{
  uint32_t loop ;
  vaddrnode_pg_manage_t *vnode_pg ;
  dlist_node_t *dlnode = NULL ;

  if( pcb == NULL )
    return NULL ;

  if( pcb->vaddrnode_alloc.ncnt != 0 )   // 有节点在存储池链表中
  {
    dlnode = pcb->vaddrnode_alloc.head.next ; // 取得第一个节点
    // 获取blobxx_pg_manage_t结构体指针
    vnode_pg = get_struct_pointor( vaddrnode_pg_manage_t, lnode, dlnode ) ; 
    // 管理节点中还有空闲内存
    if( vnode_pg->remain_cnt != 0 )
      goto vnode_pg_done ;   //直接去分配内存
  }
  // 管理节点中都没有可用内存了,分配新的管理节点
  if( mk_vaddrnode_pg_manage( pcb ) == FALSE )
    return NULL ;  //没有多余的内存了

  vnode_pg_done :     //可以分配块了
    dlnode = pcb->vaddrnode_alloc.head.next ;
    vnode_pg = get_struct_pointor( vaddrnode_pg_manage_t, lnode, dlnode ) ;

    // 查找空闲块
    for( loop = 0; loop < __INIT_BLOCK_CNT ; loop++ )
    {
      if( vnode_pg->block_array[loop].used == FALSE ) //找到空闲块
      {
        vnode_pg->block_array[loop].used = TRUE ;
        vnode_pg->remain_cnt-- ;
        //判断是否要将这个节点移动到链表尾
        if( vnode_pg->remain_cnt == 0 ) //已经没有空闲块了
          // 在链表尾插入
          add_dlnode( pcb->vaddrnode_alloc.tail.prev, del_dlnode( &(vnode_pg->lnode) ) ) ;

        return (vaddr_node_t *)&(vnode_pg->block_array[loop].block) ;
      }
    }

  return NULL ;
}


// 创建一个vaddrnode_pg_manage_t结构
// 初始化并将其插入到vaddrnode_alloc的头节点后
static bool_t
mk_vaddrnode_pg_manage( pcb_t *pcb )
{
  vaddrnode_pg_manage_t *vnode_pg ;
  uint32_t loop ;

  if( pcb == NULL )
    return FALSE ;

  uint32_t pg_addr = alloc_pages( 1 ) ;
  if( pg_addr == 0 )  // 没有可用内存了
    return FALSE ;
  
  vnode_pg = (vaddrnode_pg_manage_t *)pg_addr ;
  vnode_pg->all_cnt = __INIT_BLOCK_CNT ;
  vnode_pg->remain_cnt = __INIT_BLOCK_CNT ;
  if( init_dlnode( &(vnode_pg->lnode) ) == FALSE )
  {
    free_pages( pg_addr, 1 ) ;
    return FALSE ;
  }

  for( loop = 0 ; loop < __INIT_BLOCK_CNT ; loop ++ )
    vnode_pg->block_array[loop].used = FALSE ;

  if( add_dlnode( &(pcb->vaddrnode_alloc.head), &(vnode_pg->lnode) ) == FALSE )
  {
    free_pages( pg_addr, 1 ) ;
    return FALSE ;
  }
  return TRUE ;
}

