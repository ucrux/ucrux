#include "page.h"
#include "kernel.h"
#include "buddy_sys.h"
#include "string.h"
#include "get_struct_pointor.h"

// 传入vaddr_t的指针,因为这个只有我用,所以就不判断空指针
#define get_vaddr_end( vaddr ) \
  ( ((vaddr)->vaddr_begin) + ((vaddr)->pg_cnt * __PAGE_SIZE) - 1 )

static void *pg_getkey( const bintree_node_t *btnode ) ;
static int pg_cmp( const void *key1, const void *key2 ) ;
// 查找可用虚拟地址空间,返回可用虚拟地址空间的起始地址
// 这个函数仅针对用户空间
//static uint32_t search_uspace_vaddr( vaddr_manage_t *vaddr_manage, uint32_t pg_count ) ;


//将线性地址与物理叶帧绑定
//因为内核运行在平坦模型上
//所以,pdt_add,pte_addr,phy_pg_add都是物理地址
void
add_page( uint32_t pdt_addr, uint32_t pte_addr, \
          uint32_t phy_pg_addr, uint32_t linar_addr, uint32_t pg_attr )
{
  uint32_t *pdt = (uint32_t *)pdt_addr ;
  uint32_t *pte = (uint32_t *)pte_addr ;

  pdt[pdt_idx(linar_addr)] = (pte_addr & __PG_MASK) | pg_attr ;
  pte[pte_idx(linar_addr)] = (phy_pg_addr & __PG_MASK) | pg_attr ;
}



uint32_t
get_pg_phy_addr( uint32_t linar_addr, uint32_t pdt_addr )
{
  uint32_t pdtidx, pteidx, ptet_addr, pte, *pdt_ptr, *pte_ptr ;
  // 获取线性地址在页目录表中的索引
  pdtidx = pdt_idx( linar_addr ) ;

  // 获取页目录表的指针
  pdt_ptr = (uint32_t *)pdt_addr ;

  // 获取页表项表的指针
  ptet_addr = pdt_ptr[pdtidx] ;
  // 判断ptet是否存在
  if( ( ptet_addr & __PG_P ) == 0 ) // ptet不存在
    return 0 ;
  // 获取页目录表的索引
  pteidx = pte_idx( linar_addr ) ;
  pte_ptr = (uint32_t *)(ptet_addr & __PG_MASK) ;
  // 获取线性地址所在页珍的叶帧首地址
  pte = pte_ptr[pteidx] ;
  // 判断这个线性地址是否映射了叶帧
  if( ( pte & __PG_P ) == 0 ) // 没有映射叶帧
    return 0 ;
  return ( (pte & __PG_MASK) | (linar_addr & (~__PG_MASK)) ) ;
}

// 初始化虚拟地址管理节点
bool_t
init_vaddr_manage( vaddr_manage_t *vaddr_manage )
{
  return init_avl_tree( vaddr_manage, pg_getkey, pg_cmp ) ;
}

// 初始化虚拟地址节点
bool_t
init_vaddr_node( vaddr_node_t *vaddrnode, uint32_t vaddr_begin, uint32_t pg_cnt )
{
  if( vaddrnode == NULL )
    return FALSE ;
  vaddrnode->vaddr.vaddr_begin = vaddr_begin ;
  vaddrnode->vaddr.pg_cnt = pg_cnt ;
  return init_avl_tree_node( &(vaddrnode->avlnode) ) ;
}

// 插入虚拟地址空间
bool_t 
inst_vaddr_node( vaddr_manage_t *vaddr_manage, vaddr_node_t *vaddrnode )
{
  if( vaddr_manage == NULL || vaddrnode == NULL )
    return FALSE ;
  return insert_avl( (avltree_t *)vaddr_manage, &(vaddrnode->avlnode) ) ;
}

// 删除地址空间
vaddr_node_t *
delete_vaddr_node( vaddr_manage_t *vaddr_manage, vaddr_t *vaddr )
{
  if( vaddr_manage == NULL || vaddr == NULL )
    return NULL ;
  vaddr_node_t *vaddrnode = NULL ;
  avl_tree_node_t *avlnode = delete_avl( (avltree_t *)vaddr_manage, (void *)vaddr ) ;
  if( avlnode != NULL )
    vaddrnode = get_struct_pointor( vaddr_node_t, avlnode, avlnode ) ;

  return vaddrnode ;
}

// 寻找 vaddr 是否被 vaddr_manage 管理
vaddr_node_t *
search_vaddr_node( vaddr_manage_t *vaddr_manage, vaddr_t *vaddr )
{
  if( vaddr_manage == NULL || vaddr == NULL )
    return NULL ;
  vaddr_node_t *vaddrnode = NULL ;
  avl_tree_node_t *avlnode ;
  bintree_node_t *btnode = bintree_search( (bintree_t *)vaddr_manage, (void *)vaddr ) ;
  if( btnode == NULL )
    return NULL ;

  avlnode = get_struct_pointor( avl_tree_node_t, btnode, btnode ) ;
  vaddrnode = get_struct_pointor( vaddr_node_t, avlnode, avlnode ) ;

  return vaddrnode ;
}

// 分配虚拟地址空间,成功返回虚拟地址的首地址,失败返回0
// 此函数只用于在用户进程,
// 内核使用的是平坦模型
// 不需要搜寻可以虚拟地址空间
uint32_t 
alloc_vaddr( vaddr_manage_t *vaddr_manage, uint32_t pg_count )
{
  avl_tree_node_t *avlnode, *suc_avlnode ;
  bintree_node_t *tnode, *suc_tnode ;
  vaddr_node_t *vaddrnode, *suc_vaddrnode ;

  if( vaddr_manage->tnode_cnt == 0 ) //此时虚拟地址是空的
    return __KERNEL_MAX_ADDR ;

  // 找到此树的第一个节点
  tnode = vaddr_manage->p_root ;
  while( tnode->p_left != NULL )
    tnode = tnode->p_left ;

  // 此时 avlnode 也就是这个树中序历遍的第一个节点了
  suc_tnode = suc_bintree_node( tnode ) ;

  while( suc_tnode != NULL )
  {
    avlnode = get_struct_pointor( avl_tree_node_t, btnode, tnode ) ;
    suc_avlnode = get_struct_pointor( avl_tree_node_t, btnode, suc_tnode ) ;

    vaddrnode = get_struct_pointor( vaddr_node_t, avlnode, avlnode ) ;
    suc_vaddrnode = get_struct_pointor( vaddr_node_t, avlnode, suc_avlnode ) ;
         // 空闲的地址空间
    if( (suc_vaddrnode->vaddr.vaddr_begin - get_vaddr_end( &(vaddrnode->vaddr) ) - 1) \
         >= pg_count * __PAGE_SIZE )
      return (get_vaddr_end( &(vaddrnode->vaddr) ) + 1) ;

    tnode = suc_tnode ;
    suc_tnode = suc_bintree_node( tnode ) ;
  }

  avlnode = get_struct_pointor( avl_tree_node_t, btnode, tnode ) ;
  vaddrnode = get_struct_pointor( vaddr_node_t, avlnode, avlnode ) ;
  if( get_vaddr_end( &(vaddrnode->vaddr) ) + pg_count * __PAGE_SIZE <= __VADDR_MAX )
    return ( get_vaddr_end( &(vaddrnode->vaddr) ) + 1 ) ;
  return 0 ;
}


// 根据pdt的地址,线性地址,叶帧物理地址,对叶帧做映射
// 因为要映射的只有用户空间,所有属性就是用户属性
bool_t 
map_page( uint32_t *pdt, uint32_t linar_addr, uint32_t pg_phy_addr )
{
  uint32_t pdtidx, pteidx, ptet_addr, *pte_ptr ;
  // 获取线性地址在页目录表中的索引
  pdtidx = pdt_idx( linar_addr ) ;

  // 获取页表项表的指针
  ptet_addr = pdt[pdtidx] ;
  // 判断ptet是否存在
  if( ( ptet_addr & __PG_P ) == 0 ) // ptet不存在
  {
    // 分配一个新的ptet
    if( (ptet_addr = alloc_pages( 1 ) ) == 0 )
      return FALSE ;
    pdt[pdtidx] = ptet_addr | (__PG_RWW | __PG_USU | __PG_P ) ;
  }
  // 获取页表项表的索引
  pteidx = pte_idx( linar_addr ) ;
  pte_ptr = (uint32_t *)(ptet_addr & __PG_MASK) ;
  // 映射物理地址
  pte_ptr[pteidx] = (pg_phy_addr & __PG_MASK) | (__PG_RWW | __PG_USU | __PG_P ) ;
  
  return TRUE ;
}

// 清理指定线性地址的映射
void 
unmap_page( uint32_t *pdt, uint32_t linar_addr )
{
    uint32_t pdtidx, pteidx, ptet_addr, *pte_ptr ;
  // 获取线性地址在页目录表中的索引
  pdtidx = pdt_idx( linar_addr ) ;

  // 获取页表项表的指针
  ptet_addr = pdt[pdtidx] ;
  // 判断ptet是否存在
  if( ( ptet_addr & __PG_P ) == 0 ) // ptet不存在
    return ;
  // 获取页表项表的索引
  pteidx = pte_idx( linar_addr ) ;
  pte_ptr = (uint32_t *)(ptet_addr & __PG_MASK) ;
  // 解除映射物理地址
  pte_ptr[pteidx] = pte_ptr[pteidx] & (~__PG_P ) ;
  
}

// 创建一个新的pdt,参数为另一pdt
// 如果参数为空,就创建一个只映射低1M内存(对等映射)的pdt,即原始的内核空间
// 如果参数不为空,就复制按照传入的参数的pdt,ptet以及对应的物理叶帧
// 为了方便,这里就不使用延时复制了
// 低于1G的属于内核空间地址
// 所以不需要复制叶帧
// 成功返回 新的 pdt 的物理地址. 失败返回 NULL
uint32_t *
mk_pdt( uint32_t *oldpdt )
{
  uint32_t *newpdt, *ptet, *oldptet ;
  uint32_t linar_addr, looppdt, looppte, phy_addr ;

  newpdt = (uint32_t *)alloc_pages( 1 ) ;
  if( newpdt == NULL )  // 已经没有可用内存了
    return NULL ;

  if( oldpdt == NULL )    // 创建低1M内存对等映射的pdt机pte
  {                       // 为例创建的成功率,每次分配一页的内存
    ptet = (uint32_t *)alloc_pages( 1 ) ;
    if( ptet == NULL )    // 已经没有可用内存了
    {
      // 释放newpdt的页面,这是在做清理工作
      free_pages( (uint32_t )newpdt, 1 ) ;
      return NULL ;
    }
    // 以下做低端1M内存的对等映射
    for( linar_addr = 0 ; linar_addr < __INIT_BEGIN_ADDR; linar_addr += __PAGE_SIZE )
      add_page( (uint32_t )newpdt, \
                (uint32_t)(ptet), \
                linar_addr, \
                linar_addr, \
                __PG_P | __PG_RWW | __PG_USS ) ;
  }
  else    //以原始pdt为模版
  {
    for( looppdt = 0 ; looppdt < __PG_ENT_CNT ; looppdt++ )
    {
      if( ( (oldpdt[looppdt]) & (__PG_P) ) != 0 && looppdt >= __KERNEL_MAX_PDT_ECNT )
      {
        ptet = (uint32_t *)alloc_pages( 1 ) ;
        if( ptet == NULL )
        {
          free_pdt( newpdt ) ;
          return NULL ;
        }
        newpdt[looppdt] = ((uint32_t )ptet) | (( oldpdt[looppdt] & (~__PG_MASK) )) ;
                                              // 取原ptet的属性
        // 以下分配叶帧进行线性地址映射
        oldptet = (uint32_t *)(oldpdt[looppdt] | __PG_MASK ) ;
        for( looppte = 0 ; looppte < __PG_ENT_CNT ; looppte++ )
        {
          if( (oldptet[looppte] & __PG_P) != 0 )
          {
            phy_addr = alloc_pages( 1 ) ;
            if( phy_addr == 0 )
            {
              free_pdt( newpdt ) ;
              return NULL ;
            }
            ptet[looppte] = phy_addr | ( oldptet[looppte] & (~__PG_MASK) ) ;
                                       // 获取旧叶帧属性
            // 拷贝叶帧内容
            memcpy( (void *)phy_addr, (void *)(oldptet[looppte] & __PG_MASK ), __PAGE_SIZE ) ;
          }
          else  // 物理叶帧不存在
            ptet[looppte] = oldptet[looppte] ;
        }
      }
      else      // 页表项表不存在
        newpdt[looppdt] = oldpdt[looppdt] ;
    }
  }
  return newpdt ;
}


// 根据pdt的值回收所有 pdt, pte,以及所有做了映射的页表
void 
free_pdt( uint32_t *pdt )
{
  uint32_t loop_pdt, ptet_addr, loop_ptet, *ptet ;

  if( pdt == NULL )
    return ;

  for( loop_pdt = __KERNEL_MAX_PDT_ECNT ; loop_pdt < __PG_ENT_CNT ; loop_pdt++ )
  {
    ptet_addr = pdt[loop_pdt] ;
    if( (ptet_addr & __PG_P) == 0 )
      continue ;
    ptet = (uint32_t *)ptet_addr ;
    for( loop_ptet = 0 ; loop_ptet < __PG_ENT_CNT ; loop_ptet++ )
    {
      if( (ptet[loop_ptet] & __PG_P) == 0 )
        continue ;
      free_pages( (ptet[loop_ptet] & __PG_MASK) , 1 ) ;
    }
    free_pages( ptet_addr, 1 ) ;
  }

  free_pages( (uint32_t )pdt, 1 ) ;
}

// 局部函数
static void *
pg_getkey( const bintree_node_t *btnode )
{
  avl_tree_node_t *avlnode ;
  vaddr_node_t *vaddr_node ;

  if( btnode == NULL )
    return NULL ;

  avlnode = get_struct_pointor( avl_tree_node_t, btnode, btnode ) ;
  vaddr_node = get_struct_pointor( vaddr_node_t, avlnode, avlnode ) ;

  return (void *)(&(vaddr_node->vaddr)) ;
}

static int 
pg_cmp( const void *key1, const void *key2 )
{
  vaddr_t *vaddr1, *vaddr2 ;
  vaddr1 = (vaddr_t *)key1 ;
  vaddr2 = (vaddr_t *)key2 ;

  if( vaddr1 == NULL && vaddr2 == NULL )
    return 0 ;
  if( vaddr1 == NULL && vaddr2 != NULL )
    return -1 ;
  if( vaddr1 != NULL && vaddr2 == NULL )
    return 1 ;

  if( vaddr1->vaddr_begin > vaddr2->vaddr_begin )
    return 1 ;
  else if( vaddr1->vaddr_begin < vaddr2->vaddr_begin )
    return -1 ;
  else
    return 0 ;
}

