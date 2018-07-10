#ifndef __PAGE_H
#define __PAGE_H

#include "global_type.h"
#include "avl_tree.h"

#define __PG_P        1U
#define __PG_RWR      0U
#define __PG_RWW      2U
#define __PG_USS      0U
#define __PG_USU      4U
#define __PDE_MASK    0xffc00000U
#define __PTE_MASK    0x003ff000U
#define __PG_MASK     0xfffff000U
#define __PG_ENT_CNT  (__PAGE_SIZE / 4U)


// 根据线性地址获取在页目录表中的索引
#define pdt_idx( linar_addr ) \
          ( ((linar_addr) & __PDE_MASK) >> 22 )
// 根据线性地址获取在页表项表中的索引
#define pte_idx( linar_addr ) \
          ( ((linar_addr) & __PTE_MASK) >> 12 )

// 虚拟地址管理数据结构
typedef avltree_t vaddr_manage_t ;
// 虚拟地址起始数据结构,这个作为平衡树的关键字,
// 只要两个节点的虚拟地址有交集就算相等
typedef struct s_vaddr
{
  uint32_t vaddr_begin ;
  uint32_t pg_cnt ;
} vaddr_t ;

// 虚拟地址节点
typedef struct s_vaddr_node
{
  vaddr_t vaddr ;
  avl_tree_node_t avlnode ;
} vaddr_node_t ;


//将线性地址与物理叶帧绑定
//因为内核运行在平坦模型上
//所以,pdt_add,pte_addr,phy_pg_add都是物理地址
void add_page( uint32_t pdt_addr, uint32_t pte_addr, \
               uint32_t phy_pg_addr, uint32_t linar_addr, uint32_t pg_attr ) ;


// 根据线性地址获取页表的物理地址
// 此函数只会在内核空间调用
// 所以运行此函数的时候可能要换上内核页目录表
// 内核运行在平坦模型上,所以 pdt_addr使用的是物理地址
uint32_t get_pg_phy_addr( uint32_t linar_addr, uint32_t pdt_addr ) ;


// 初始化虚拟地址管理节点
bool_t init_vaddr_manage( vaddr_manage_t *vaddr_manage ) ;
// 初始化虚拟地址节点
bool_t init_vaddr_node( vaddr_node_t *vaddrnode, uint32_t vaddr_begin, uint32_t pg_cnt ) ;
// 插入虚拟地址空间
bool_t inst_vaddr_node( vaddr_manage_t *vaddr_manage, vaddr_node_t *vaddrnode ) ;
// 删除地址空间
vaddr_node_t *delete_vaddr_node( vaddr_manage_t *vaddr_manage, vaddr_t *vaddr ) ;
// 寻找 vaddr 是否被 vaddr_manage 管理
vaddr_node_t *search_vaddr_node( vaddr_manage_t *vaddr_manage, vaddr_t *vaddr ) ;
// 分配虚拟地址空间,成功返回虚拟地址的首地址,失败返回0
// 此函数只用于在用户进程,
// 内核使用的是平坦模型
// 不需要搜寻可以虚拟地址空间
uint32_t alloc_vaddr( vaddr_manage_t *vaddr_manage, \
                    uint32_t pg_count ) ;

// 根据pdt的地址,线性地址,叶帧物理地址,对叶帧做映射
// 因为要映射的只有用户空间,所有属性就是用户属性
bool_t map_page( uint32_t *pdt, uint32_t linar_addr, uint32_t pg_phy_addr ) ;
// 清理指定线性地址的映射
void unmap_page( uint32_t *pdt, uint32_t linar_addr ) ;

/*
 * 分配和释放pdt请使用如下函数
 * 因为pdt及ptet所使用的内存不受内核管理 
 */

// 创建一个新的pdt,参数为另一pdt
// 如果参数为空,就创建一个只映射低1M内存(对等映射)的pdt,即原始的内核空间
// 如果参数不为空,就复制按照传入的参数的pdt,ptet以及对应的物理叶帧
// 为了方便,这里就不使用延时复制了
// 低于1G的属于内核空间地址
// 所以不需要复制叶帧
// 成功返回 新的 pdt 的物理地址. 失败返回 NULL
uint32_t *mk_pdt( uint32_t *oldpdt ) ;
// 根据pdt的值回收所有 pdt,pte,以及所有做了映射的页表
void free_pdt( uint32_t *pdt ) ;

#endif