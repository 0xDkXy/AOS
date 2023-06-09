#include "debug.h"
#include "kernel/print.h"
#include "interrupt.h"

/* print filename, number of line, function name, condition and spin
 * */

#define NEWLINE put_str("\n");

void panic_spin(char* filename, \
                int line,   \
                const char* func, \
                const char* condition)
{
    intr_disable();
    
    put_str("\n\n\n!!!!! error !!!!!\n");
    put_str("filename:"); put_str(filename); NEWLINE
    put_str("line:0x"); put_int(line);  NEWLINE
    put_str("function:"); put_str((char*)func);  NEWLINE
    put_str("condition:"); put_str((char*)condition); NEWLINE
    while(1);
}
