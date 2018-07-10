#ifndef __VIDEO_H
#define __VIDEO_H

// video 服务进程别名
#define __VIDEO_TASK_ALIAS        "video"

// video 功能号
#define __VIDEO_FUN_PRINT_STRING  0U
#define __VIDEO_FUN_PRINT_CHAR    1U
#define __VIDEO_FUN_PRINT_NUM_HEX 2U
#define __VIDEO_FUN_PRINT_NUM_DEC 3U

// msg_buff  保存要打印的东西
// 字符串最大打印 1536 个 (包含'\0')


#endif
