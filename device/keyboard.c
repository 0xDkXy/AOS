#include "keyboard.h"
#include "kernel/print.h"
#include "interrupt.h"
#include "kernel/io.h"
#include "global.h"

#define KBD_BUF_PORT 0x60 // the keboard buffer reg port

static void intr_keyboard_handler(void)
{
    uint8_t scanCode = inb(KBD_BUF_PORT);
    put_int(scanCode);
    return;
}

void keyboard_init()
{
    put_str("keyboard init start\n");
    register_handler(0x21, intr_keyboard_handler);
    put_str("keyboard init done\n");
}
