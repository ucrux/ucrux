#ifndef __GET_STRUCT_POINTOR_H
#define __GET_STRUCT_POINTOR_H


#define get_struct_pointor( struct_name_t, elem_name, elem_inner_pointer ) \
  (struct_name_t *)\
  ( \
    (char *)(elem_inner_pointer) - \
    (char *)( &((struct_name_t *)(0) )->elem_name ) \
    )

#endif