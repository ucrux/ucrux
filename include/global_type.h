#ifndef __GLOBAL_TYPE_H
#define __GLOBAL_TYPE_H

// 证书相关数据类型
typedef signed char int8_t ;
typedef signed short int int16_t ;
typedef signed int int32_t ;
typedef signed long long int int64_t ;
typedef unsigned char uint8_t ;
typedef unsigned short int uint16_t ;
typedef unsigned int uint32_t ;
typedef unsigned long long int uint64_t ;


// 空指针
#define NULL ((void *)0)

// bool类型定义
typedef uint32_t bool_t ;
#define  TRUE   1
#define  FALSE  0
#define  ERROR  -1

// 整数类型长度,以byte为单位
#define __UINT64_LEN  8
#define __INT64_LEN   (__UINT64_LEN)
#define __UINT32_LEN  4
#define __INT32_LEN   (__UINT32_LEN)
#define __UINT16_LEN  2
#define __INT16_LEN   (__UINT16_LEN)
#define __UINT8_LEN   1
#define __INT8_LEN    (__UINT8_LEN)

typedef enum e_msg_type
{
  INTR,
  KERNEL,
  //TASK,
  PROC,
  MSG_TYPE_CNT
} msg_type_t ;

#endif