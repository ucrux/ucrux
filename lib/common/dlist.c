#include "dlist.h"

bool_t
init_dlist( dlist_head_t *dlhead )
{
  if( dlhead == NULL )
    return FALSE ;

  dlhead->head.next = &(dlhead->tail) ;
  dlhead->head.prev = &(dlhead->tail) ;
  dlhead->head.dlhead = dlhead ;

  dlhead->tail.next = &(dlhead->head) ;
  dlhead->tail.prev = &(dlhead->head) ;
  dlhead->tail.dlhead = dlhead ;

  dlhead->ncnt = 0 ;

  return TRUE ;
}

bool_t
init_dlnode( dlist_node_t *dlnode )
{
  if( dlnode == NULL )
    return FALSE ;

  dlnode->dlhead = NULL ;
  dlnode->next = NULL ;
  dlnode->prev = NULL ;
  return TRUE ;
}


dlist_node_t *
del_dlnode( dlist_node_t *dlnode )
{
  if( dlnode == NULL )
    return NULL ;

  if( dlnode->dlhead == NULL || dlnode->prev == NULL || dlnode->next == NULL )
    return NULL ;

  //删除了dlhead中的节点,非法操作
  if( &(dlnode->dlhead->head) == dlnode || &(dlnode->dlhead->tail) == dlnode )
    return NULL ;

  dlnode->prev->next = dlnode->next ;
  dlnode->next->prev = dlnode->prev ;

  dlnode->dlhead->ncnt-- ;

  dlnode->dlhead = NULL ;
  dlnode->prev = dlnode->next = NULL ;

  return dlnode ;
}


//after prev,add new
bool_t
add_dlnode( dlist_node_t *prev, dlist_node_t *new )
{
  if( prev == NULL || new == NULL || \
      prev->dlhead == NULL || prev->prev == NULL || prev->next == NULL || \
      new->prev != NULL || new->next != NULL || new->dlhead != NULL )
    return FALSE ;

  new->next = prev->next ;
  new->prev = prev ;

  prev->next->prev = new ;
  prev->next = new ;

  new->dlhead = prev->dlhead ;

  new->dlhead->ncnt++ ;

  return TRUE ;
}


dlist_node_t *
trav_dlist( dlist_head_t *dlhead, bool_t (*travfun)( void *arg1, const dlist_node_t *arg2 ),  void *arg1 )
{
  dlist_node_t *tmpnode ;
  if( dlhead == NULL || travfun == NULL || arg1 == NULL )
    return NULL ;
  for_each_dlnode( dlhead, tmpnode )
  {
    if( travfun( arg1, tmpnode ) )
      return tmpnode ;
  }

  return NULL ;
}