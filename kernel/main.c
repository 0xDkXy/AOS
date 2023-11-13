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
#include "dir.h"


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

    printf("\n/dir1 content before delete /dir1/subdir1:\n");
    struct dir* dir = sys_opendir("/dir1/");
    char* type = NULL;
    struct dir_entry* dir_e = NULL;
    while ((dir_e = sys_readdir(dir))) {
        type = dir_e->f_type == FT_REGULAR ? "regular" : "directory";
        printf("    %s  %s\n", type, dir_e->filename);
    }
    printf("\ntry to delete nonempty directory /dir1/subdir1\n");
    if (sys_rmdir("/dir1/subdir1") == -1) {
        printf("sys_rmdir: /dir1/subdir1 delete failed!\n");
    }

    printf("\ntry to delete /dir1/subdir1/file2\n");
    if (sys_rmdir("/dir1/subdir1/file2") == -1) {
        printf("sys_rmdir: /dir1/subdir1/file2 delete failed!\n");
    }

    if (sys_unlink("/dir1/subdir1/file2") == 0) {
        printf("sys_unlink: /dir1/subdir1/file2 delete done!\n");
    }

    printf("\ntry to delete directory /dir1/subdir1 again\n");
    if (sys_rmdir("/dir1/subdir1") == 0) {
        printf("/dir1/subdir1 delete failed!\n");
    }

    printf("/dir1 content after delete /dir1/subdir1:\n");
    sys_rewinddir(dir);
    while ((dir_e = sys_readdir(dir))) {
        type = dir_e->f_type == FT_REGULAR ? "regular" : "directory";
        printf("    %s  %s\n", type, dir_e->filename);
    }


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
