#include "init.h"
#include "kernel/print.h"
#include "memory.h"
#include "interrupt.h"
#include "../device/timer.h"
#include "thread.h"

void init_all()
{
    put_str("init_all\n");
    idt_init();
    timer_init();
    mem_init();
    thread_init();
    console_init();
}
