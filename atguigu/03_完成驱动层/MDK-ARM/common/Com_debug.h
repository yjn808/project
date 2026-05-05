#ifndef __COM_DEBUG_H
#define __COM_DEBUG_H

// 使用串口重定义实现日志输出打印
#include "usart.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
// 日志输出打印的总开关 => 方便替换到生成环境  关闭所有的日志打印
#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE

// 使用字符串分割函数  去掉__FILE__中的目录名
// strrchr在一个字符串中反向查找char
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FILENAME (strrchr(__FILENAME__, '\\') ? strrchr(__FILENAME__, '\\') + 1 : __FILENAME__)

#define debug_printf(format, ...) printf("[%s:%d]" format "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

#else
// 定义一个空文本替换宏定义
#define debug_printf(format, ...)

#endif
#endif // !__COM_DEBUG_H
