#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include "stdint.h"

typedef char* va_list;
uint32_t printf(const char* format, ...);

#define va_start(ap, v) ap = (va_list)&v
#define va_arg(ap, t) *((t*)(ap += 4))
#define va_end(ap) ap=NULL

#endif // __LIB_STDIO_H
