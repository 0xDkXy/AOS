#include "kernel/print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"

void k_thread_a(void*);
void k_thread_b(void*);

int main(void)
{
    put_str("I am kernel\n");
    init_all();

    // intr_disable();

    thread_start("k_thread_a", 32, k_thread_a, "argA ");
    thread_start("k_thread_b", 8, k_thread_b, "argB ");

    intr_enable();
    // void* new_page = get_kernel_pages(1);
    // put_str("new page addr: ");
    // put_int((uint32_t)(new_page));
    // put_str("\n");

    // enum intr_status status = intr_get_status();
    // if (status == INTR_ON)
    //     put_str("intr on \n");
    // else
    //     put_str("intr off\n");


    while(1) {
        console_put_str("Main ");
    }
    return 0;
}

void k_thread_a(void* arg)
{
    char* para = arg;
    while (1) {
        console_put_str(para);
    }
}

void k_thread_b(void* arg)
{
    char* para = arg;
    while (1) {
        console_put_str(para);
    }
}
