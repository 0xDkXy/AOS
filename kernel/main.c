#include "kernel/print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "process.h"
#include "printk.h"
#include "user/syscall.h"
#include "syscall-init.h"
#include "stdio.h"


int main(void);
void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0, prog_b_pid = 0;

int main(void)
{
    put_str("I am kernel\n");
    init_all();

    // thread_start("k_thread_a", 31, k_thread_a, " A_");
    // thread_start("k_thread_b", 31, k_thread_b, " B_");
    // process_execute(u_prog_a, "user_prog_a");
    // process_execute(u_prog_b, "user_prog_b");

    intr_enable();

    // console_put_str("main_pid: 0x");
    // console_put_int(sys_getpid());
    // console_put_str("\n");

    thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
    thread_start("k_thread_b", 31, k_thread_b, "I am thread_b");

    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    char* para = arg;
    void* addr = sys_malloc(33);
    console_put_str("I am thread_a, sys_malloc(33), addr is 0x");
    console_put_int((int)addr);
    console_put_char('\n');
    while(1);
}

void k_thread_b(void* arg)
{
    char* para = arg;
    void* addr = sys_malloc(63);
    console_put_str("I am thread_a, sys_malloc(63), addr is 0x");
    console_put_int((int)addr);
    console_put_char('\n');
    while(1);
}

void u_prog_a(void)
{
    printf("prog_a_pid: 0x%x\n", getpid());
    while(1);
}

void u_prog_b(void)
{
    printf("prog_b_pid: 0x%x\n", getpid());
    while(1);
}
