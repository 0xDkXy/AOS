#include "fs.h"
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
#include "user/syscall.h"
#include "syscall-init.h"
#include "stdio.h"
#include "stdio-kernel.h"
#include "string.h"


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

    // process_execute(u_prog_a, "user_prog_a");
    // process_execute(u_prog_b, "user_prog_b");
    // thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
    // thread_start("k_thread_b", 31, k_thread_b, "I am thread_b");

    // uint32_t fd = sys_open("/file2", O_CREAT);
    // printk("fd: %d\n", fd);
    // sys_close(fd);
    // printk("%d closed now\n", fd);

    // uint32_t fd = sys_open("/file2", O_RDWR);
    // printk("fd: %d\n", fd);
    // sys_write(fd, "hello,world\n", 12);
    // sys_close(fd);
    // printk("%d closed now\n", fd);


    uint32_t fd = sys_open("/file2", O_RDWR);
    printf("open /file2, fd: %d\n", fd);
    char buf[64] = {0};
    int read_bytes = sys_read(fd, buf, 18);
    printf("1_ read %d bytes:\n%s\n", read_bytes, buf);

    memset(buf, 0, 64);
    read_bytes = sys_read(fd, buf, 6);
    printf("2_ read %d bytes:\n%s\n", read_bytes, buf);

    memset(buf, 0, 64);
    read_bytes = sys_read(fd, buf, 6);
    printf("3_ read %d bytes:\n%s\n", read_bytes, buf);

    printf("____ SEEK_SET 0 ----\n");
    sys_lseek(fd, 0 ,SEEK_SET);
    memset(buf, 0 ,64);
    read_bytes = sys_read(fd, buf, 24);
    printf("4_ read %d bytes:\n%s\n", read_bytes, buf);

    sys_close(fd);

    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    uint32_t i = 500;
    while (i-- > 1) {
        printk("A: Times: %d\n", i);
        void* addr1 = sys_malloc(i);
        void* addr2 = sys_malloc(i);
        void* addr3 = sys_malloc(i);
        thread_yield();
        sys_free(addr1);
        sys_free(addr2);
        sys_free(addr3);
    }
    printk("test A done\n");
    while(1);
}

void k_thread_b(void* arg)
{
    uint32_t i = 500;
    while (i-- > 1) {
        printk("B: Times: %d\n", i);
        void* addr1 = sys_malloc(i);
        void* addr2 = sys_malloc(i);
        void* addr3 = sys_malloc(i);
        thread_yield();
        sys_free(addr1);
        sys_free(addr2);
        sys_free(addr3);
    }
    printk("test B done\n");
    while(1);
}

void u_prog_a(void)
{
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_a malloc addr: 0x%x, 0x%x, 0x%x\n", (uint32_t)addr1,(uint32_t)addr2,(uint32_t)addr3);

    int cpu_delay = 100000;
    while (cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}

void u_prog_b(void)
{
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_a malloc addr: 0x%x, 0x%x, 0x%x\n", (uint32_t)addr1,(uint32_t)addr2,(uint32_t)addr3);

    int cpu_delay = 100000;
    while (cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}
