#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "stdint.h"

void console_init();
void console_acquire();
void console_release();
void console_put_str(char*);
void console_put_char(uint8_t);
void console_put_int(uint32_t);
void sys_putchar(char chr);

#endif // __CONSOLE_H
