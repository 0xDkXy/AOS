#include "init.h"
#include "kernel/print.h"
#include "memory.h"
#include "interrupt.h"

void init_all()
{
    put_str("init_all\n");
    idt_init();
    mem_init();
}
