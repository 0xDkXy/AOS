#include "kernel/print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "keyboard.h"

void k_thread_a(void*);
void k_thread_b(void*);

int main(void)
{
    put_str("I am kernel\n");
    init_all();

    // intr_disable();

    thread_start("k_thread_a", 31, k_thread_a, " A_");
    thread_start("k_thread_b", 31, k_thread_b, " B_");

    intr_enable();

    // enum intr_status status = intr_get_status();
    // if (status == INTR_ON)
    //     put_str("intr on \n");
    // else
    //     put_str("intr off\n");


    // while(1) {
    //     console_put_str("Main ");
    // }
    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    char* para = arg;
    while (1) {
        enum intr_status old_status = intr_disable();
        if (!ioq_empty(&kbd_buf)) {
            console_put_str(para);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}

void k_thread_b(void* arg)
{
    char* para = arg;
    while (1) {
        enum intr_status old_status = intr_disable();
        if (!ioq_empty(&kbd_buf)) {
            console_put_str(para);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}
