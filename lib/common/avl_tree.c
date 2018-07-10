#include "avl_tree.h"
#include "get_struct_pointor.h"


//对 avl_node 节点进行左平衡.成功返回TRUE,否则返回FALSE
static bool_t left_balance( avltree_t *avl_tree, avl_tree_node_t *avl_node ) ;
//对 avl_node 节点进行右平衡.成功返回TRUE,否则返回FALSE
static bool_t right_balance( avltree_t *avl_tree, avl_tree_node_t *avl_node ) ;
//用来插入节点,proot是root的父节点
//avl_node是要插入的节点
//taller用来判断数的高度是否有增加
static bool_t avl_balance_insert( avl_tree_node_t *proot, \
  bintree_node_t **root, avl_tree_node_t *avl_node, \
  bool_t *taller, avltree_t *avl_tree ) ;
//用来删除节点
static bool_t avl_remove_node( avl_tree_node_t *proot, \
  bintree_node_t **root, void *key , \
  bool_t *shorter, avltree_t *avl_tree ) ;
// 得到被处理删除节点的准确位置
// 如果需要被删除的节点只有一个子节点,返回此节点
// 如果需要被删除的节点有两个子节点,返回其后继交换
static avl_tree_node_t *get_del_node( avl_tree_node_t *avl_node ) ;


//全局函数
//==================
//初始化二叉平衡树,*avl_tree的内存空间由调用者管理
bool_t
init_avl_tree( avltree_t *avl_tree, \
  void *(*getkey)( const bintree_node_t *btnode ), \
  int (*cmp)( const void *key1, const void *key2 ) )
{
  if( avl_tree == NULL )
    return FALSE ;
  avl_tree->p_root = NULL ;
  avl_tree->tnode_cnt = 0 ;
  avl_tree->get_key = getkey ;
  avl_tree->compare = cmp ;

  return TRUE ;
}

//==================
//创建二叉平衡树节点, avl_node所指向的内存空间由调用者管理
bool_t
init_avl_tree_node( avl_tree_node_t *avl_node )
{
  if( avl_node == NULL )
    return FALSE ;
  avl_node->balance = EQU_HEIGHT ;

  avl_node->btnode.p_left = NULL ;
  avl_node->btnode.p_right = NULL ;
  avl_node->btnode.p_parent = NULL ;

  return TRUE ;
}

//==================
//插入节点
bool_t
insert_avl( avltree_t *avl_tree, avl_tree_node_t *avl_node )
{
  bool_t taller = FALSE ;
  if( avl_tree == NULL || avl_node == NULL )
    return FALSE ;
  // 调用函数,插入节点
  if (avl_balance_insert( NULL, \
                          &(avl_tree->p_root), \
                          avl_node, \
                          &taller, \
                          avl_tree ) )
  {
    // 插入成功,平衡树的节点数加1
    avl_tree->tnode_cnt++ ;
    return TRUE ;
  }

  return FALSE ;
}

//==================
//删除节点
avl_tree_node_t *
delete_avl( avltree_t *avl_tree, void *key )
{
  bool_t shorter = FALSE ;
  avl_tree_node_t *avl_node, *del_avl_node ;
  // 根据关键之找到 bintree_node_t 节点
  bintree_node_t *tmp ;

  if( avl_tree == NULL || key == NULL )
    return NULL ;
  
  tmp = bintree_search( avl_tree, key ) ;
  if( tmp == NULL ) // 关键字不存在直接返回
    return NULL ;
 
  // 根据关键字的查询结果获得 avl_tree_node_t 结构
  avl_node = get_struct_pointor( avl_tree_node_t, btnode, tmp ) ;
  // 获取要被删除的的节点
  del_avl_node = get_del_node( avl_node ) ;
  // 删除 del_avl_node 节点
  if( !avl_remove_node( NULL, \
                       &(avl_tree->p_root), \
                       avl_tree->get_key( &(del_avl_node->btnode) ), \
                       &shorter, \
                       avl_tree ) )
    return NULL ;
  // 删除成功,平衡树的节点数减一
  avl_tree->tnode_cnt-- ;

  //如果del_avl_node 和 avl_node 不是同一个节点,则需要吧avl_node换出来
  if( del_avl_node != avl_node )
  {
    //替换父节点
    del_avl_node->btnode.p_parent = avl_node->btnode.p_parent ;
    if( avl_node->btnode.p_parent != NULL )
    {
      if( avl_node->btnode.p_parent->p_left == &(avl_node->btnode) )
        avl_node->btnode.p_parent->p_left = &(del_avl_node->btnode) ;
      else
        avl_node->btnode.p_parent->p_right = &(del_avl_node->btnode) ;
    }
    else
      avl_tree->p_root = &(del_avl_node->btnode) ;
    //替换左节点
    del_avl_node->btnode.p_left = avl_node->btnode.p_left ;
    if( avl_node->btnode.p_left != NULL )
      avl_node->btnode.p_left->p_parent = &(del_avl_node->btnode) ;
    //替换右节点
    del_avl_node->btnode.p_right = avl_node->btnode.p_right ;
    if( avl_node->btnode.p_right != NULL )
      avl_node->btnode.p_right->p_parent = &(del_avl_node->btnode) ;
    //交换平衡因子
    del_avl_node->balance = avl_node->balance ;
  }


  // 清空 avl_node 的各个指针
  avl_node->btnode.p_left = NULL ;
  avl_node->btnode.p_right = NULL ;
  avl_node->btnode.p_parent = NULL ;
  avl_node->balance = EQU_HEIGHT ;

  return avl_node ;
}


//---------------
static bool_t
left_balance( avltree_t *avl_tree, avl_tree_node_t *avl_node )
{
  avl_tree_node_t *nl = avl_get_left_child( avl_node ) ;
  avl_tree_node_t *nlr ;
  switch ( nl->balance )
  {
    case LEFT_HIGH : 
      nl->balance = EQU_HEIGHT ;                                    /*          n             nl                         */
      avl_node->balance = EQU_HEIGHT ;                              /*         /             /  \                        */
      if( right_rotate( avl_tree, &(avl_node->btnode) ) == FALSE )  /*        nl      ==>   lh   n                       */
        return FALSE ;                                              /*       /                                           */
      break ;                                                       /*      lh                                           */
    case RIGHT_HIGH :                                               /*                                                   */
      nlr = avl_get_right_child( nl ) ;                             /*          n              n              nlr        */
      switch ( nlr->balance )                                       /*         / \            / \            /   \       */
      {                                                             /*        nl  0          nlr 0          nl    n      */
        case LEFT_HIGH :                                            /*       /  \     ==>    /        ==>  /  \    \     */
          nl->balance = EQU_HEIGHT ;                                /*      0   nlr         nl            0   lh    0    */
          avl_node->balance = RIGHT_HIGH ;                          /*          /          /  \                          */
          break ;                                                   /*         lh         0   lh                         */
        case EQU_HEIGHT :                                           /*                                                   */
          nl->balance = EQU_HEIGHT ;                                /*          n               n              nlr       */
          avl_node->balance = EQU_HEIGHT ;                          /*         /               /              /   \      */
          break ;                                                   /*        nl       ==>    nlr     ==>    nl    n     */
        case RIGHT_HIGH :                                           /*          \            /                           */
          nl->balance = LEFT_HIGH ;                                 /*          nlr         nl                           */
          avl_node->balance = EQU_HEIGHT ;                          /*                                                   */
          break ;                                                   /*          n                n              nlr      */
      }                                                             /*         / \              / \            /   \     */
      nlr->balance = EQU_HEIGHT ;                                   /*        nl  0            nlr 0          nl    n    */
      if( left_rotate( avl_tree, &(nl->btnode) ) == FALSE )         /*       /  \       ==>   /  \     ==>   /     / \   */
        return FALSE ;                                              /*      0   nlr          nl  rh         0     rh  0  */
      if( right_rotate( avl_tree, &(avl_node->btnode) ) == FALSE )  /*             \        /                            */
        return FALSE ;                                              /*             rh      0                             */
      break ;                                                       /*                                                   */
    case EQU_HEIGHT :                                               /*          n                nl                      */
      nl->balance = RIGHT_HIGH ;                                    /*         /                /  \                     */
      if( right_rotate( avl_tree, &(avl_node->btnode) ) == FALSE )  /*        nl         ==>   0    n                    */
        return FALSE ;                                              /*       /  \                  /                     */
      break ;                                                       /*      0    0                0                      */
  }                                               
  return TRUE ;                                     
}                                                 
                                                  
                                                  
//---------------
// 与 left_balance 完全对称
static bool_t
right_balance( avltree_t *avl_tree, avl_tree_node_t *avl_node )
{
  avl_tree_node_t *nr = avl_get_right_child( avl_node ) ;
  avl_tree_node_t *nrl ;
  switch ( nr->balance )
  {
    case RIGHT_HIGH :
      nr->balance = EQU_HEIGHT ;
      avl_node->balance = EQU_HEIGHT ;
      if( left_rotate( avl_tree, &(avl_node->btnode) ) == FALSE )
        return FALSE ;
      break ;
    case LEFT_HIGH :
      nrl = avl_get_left_child( nr ) ;
      switch ( nrl->balance )
      {
        case RIGHT_HIGH :
          nr->balance = EQU_HEIGHT ;
          avl_node->balance = LEFT_HIGH ;
          break ;
        case EQU_HEIGHT :
          nr->balance = EQU_HEIGHT ;
          avl_node->balance = EQU_HEIGHT ;
          break ;
        case LEFT_HIGH :
          nr->balance = RIGHT_HIGH ;
          avl_node->balance = EQU_HEIGHT ;
          break ;
      }
      nrl->balance = EQU_HEIGHT ;
      if( right_rotate( avl_tree, &(nr->btnode) ) == FALSE )
        return FALSE ;
      if( left_rotate( avl_tree, &(avl_node->btnode) ) == FALSE )
        return FALSE ;
      break ;
    case EQU_HEIGHT :
      nr->balance = LEFT_HIGH ;
      if( left_rotate( avl_tree, &(avl_node->btnode) ) == FALSE )
        return FALSE ;
      break ;
  } 
  return TRUE ;
}

//---------------
static bool_t
avl_balance_insert( avl_tree_node_t *proot, \
  bintree_node_t **root, avl_tree_node_t *avl_node, \
  bool_t *taller, avltree_t *avl_tree )
{
  int cmpresult ;               //关键字比较结果
  avl_tree_node_t *avl_root ;   //存储 bintree_node_t **root 转化为 avl_tree_node_t 结构

  if( *root == NULL )            //此时树为空,或者在找到该插入的位置
  {
    *taller = TRUE ;              //插入节点,树增高为true
    if( proot != NULL )
      avl_node->btnode.p_parent = &(proot->btnode) ;
    else
      avl_node->btnode.p_parent = NULL ;
    *root = &(avl_node->btnode) ; //将新节点插入到正确的位置
    return TRUE ;                 //插入成功
  }
  else
  {
    // 比较关键字
    avl_root = get_struct_pointor( avl_tree_node_t, btnode, (*root) ) ;
    cmpresult = avl_tree->compare( avl_tree->get_key( &(avl_root->btnode) ), \
                                   avl_tree->get_key( &(avl_node->btnode) ) ) ;
    if( cmpresult == 0 ) //不允许有重复值插入
      return FALSE ;
    // avl_root的关键字比要插入节点的关键字大,往左子树插
    else if( cmpresult == 1 )
    {
      if( avl_balance_insert( avl_root, &((*root)->p_left), avl_node, taller, avl_tree ) )
      {   
        if( *taller )
        {
          switch ( avl_root->balance )
          {
            case LEFT_HIGH : 
              left_balance( avl_tree, avl_root ) ;
              *taller = FALSE ;
              break ;
            case EQU_HEIGHT : 
              avl_root->balance = LEFT_HIGH ;
              *taller = TRUE ;
              break ;
            case RIGHT_HIGH : 
              avl_root->balance = EQU_HEIGHT ;
              *taller = FALSE ;
              break ;
          }
        }
      return TRUE ;
      }
    }
    else
    {
      if( avl_balance_insert( avl_root, &((*root)->p_right), avl_node, taller, avl_tree ) )
      {
        if( *taller )
        {
          switch ( avl_root->balance )
          {
            case RIGHT_HIGH :
              right_balance( avl_tree, avl_root ) ;
              *taller = FALSE ;
              break ;
            case EQU_HEIGHT :
              avl_root->balance = RIGHT_HIGH ;
              *taller = TRUE ;
              break ;
            case LEFT_HIGH :
              avl_root->balance = EQU_HEIGHT ;
              *taller = FALSE ;
              break ;
          }
        }
      }
      return TRUE ;
    }
  }
  return FALSE ;
}

//---------------
static avl_tree_node_t *
get_del_node( avl_tree_node_t *avl_node )
{
    avl_tree_node_t *suc_node ;
    bintree_node_t *tmp ;

    if( avl_node->btnode.p_left == NULL || avl_node->btnode.p_right == NULL )
      suc_node = avl_node ;
    else
    {
      tmp = suc_bintree_node( &(avl_node->btnode) ) ;       //这个函数在标准二叉树库里面
      suc_node = get_struct_pointor( avl_tree_node_t, btnode, tmp ) ;
    }

    return suc_node ;
}

//---------------
static bool_t
avl_remove_node( avl_tree_node_t *proot, \
  bintree_node_t **root, void *key , \
  bool_t *shorter, avltree_t *avl_tree )
// get_del_node 必须先被执行
{   //由于调用这个函数的时候已经确保*root不为NULL,所以这里可以省掉这个检查
  avl_tree_node_t *avl_root ;
  int cmpresult ;
  bintree_node_t *btntmp = NULL ;

  avl_root = get_struct_pointor( avl_tree_node_t, btnode, (*root) ) ;
  cmpresult = avl_tree->compare( avl_tree->get_key( &(avl_root->btnode) ), \
                                 key ) ;
  // 找到了关键字,从数中删除节点
  if( cmpresult == 0 )
  {
    // 节点要被删除,数当然变矮
    *shorter = TRUE ;

    // root 最多有一个子节点
    if( (*root)->p_left != NULL )
      btntmp = (*root)->p_left ;
    else
      btntmp = (*root)->p_right ;


    if( proot != NULL )
    {
      if( btntmp != NULL )
        btntmp->p_parent = &(proot->btnode) ;
      if( proot->btnode.p_left == *root )
        proot->btnode.p_left = btntmp ;
      else
        proot->btnode.p_right = btntmp ;
    }
    else
    {
      if( btntmp != NULL )
        btntmp->p_parent = NULL ;
      avl_tree->p_root = btntmp ;
    }

    return TRUE ;
  }
  else if( cmpresult == 1 ) //key的值比较小
  {
    if( avl_remove_node( avl_root, &((*root)->p_left), key, shorter, avl_tree ) )
    {
      if( *shorter )
      {
        switch ( avl_root->balance )
        {
          case LEFT_HIGH : 
            avl_root->balance = EQU_HEIGHT ;
            *shorter = TRUE ;
            break ;
          case EQU_HEIGHT :
            avl_root->balance = RIGHT_HIGH ;
            *shorter = FALSE ;
            break ;
          case RIGHT_HIGH : 
            right_balance( avl_tree, avl_root ) ;
            *shorter = TRUE ;
            break ;
        }
      }
      return TRUE ;
    }
  }
  else // key的值比较大
  {
    if( avl_remove_node( avl_root, &((*root)->p_right), key, shorter, avl_tree ) )
    {
      if( *shorter )
      {
        switch ( avl_root->balance )
        {
          case RIGHT_HIGH :
            avl_root->balance = EQU_HEIGHT ;
            *shorter = TRUE ;
            break ;
          case EQU_HEIGHT : 
            avl_root->balance = LEFT_HIGH ;
            *shorter = FALSE ;
            break ;
          case LEFT_HIGH :
            left_balance( avl_tree, avl_root ) ;
            *shorter = TRUE ;
            break ;
        }
      }
      return TRUE ;
    }
  }
  return FALSE ;
}

