#include "init.h"
#include "kernel/print.h"
#include "memory.h"
#include "interrupt.h"
#include "../device/timer.h"
#include "thread.h"
#include "../device/keyboard.h"
#include "tss.h"
#include "syscall-init.h"
#include "thread.h"
#include "console.h"
#include "ide.h"

void init_all()
{
    put_str("init_all\n");
    idt_init();
    timer_init();
    mem_init();
    thread_init();
    console_init();
    keyboard_init();
    tss_init();
    syscall_init();
    ide_init();
}
