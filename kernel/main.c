#include "fs.h"
#include "ide.h"
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
#include "shell.h"
#include "user/assert.h"


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

    /*
    struct stat obj_stat;
    sys_stat("/", &obj_stat);
    printf("/ info:\n   i_no: %d\n  size: %d\n  filetype: %s\n",
            obj_stat.st_ino, obj_stat.st_size, 
            obj_stat.st_filetype == 2 ? "directory" : "regular");

    sys_stat("/dir1", &obj_stat);
    printf("/dir1 info:\n   i_no: %d\n  size: %d\n  filetype: %s\n",
            obj_stat.st_ino, obj_stat.st_size, 
            obj_stat.st_filetype == 2 ? "directory" : "regular");
    */

    uint32_t file_size = 15560;
    uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
    struct disk* sda = &channels[0].devices[0];
    void* prog_buf = sys_malloc(file_size);
    ide_read(sda, 300, prog_buf, sec_cnt);
    int32_t fd = sys_open("/prog_arg", O_CREAT|O_RDWR);
    if (fd != -1) {
        int _byte = sys_write(fd, prog_buf, file_size);
        if (_byte == -1) {
            printk("file write error!\n");
            while(1);
        } else if (_byte != file_size) {
            printk("wrong size\n");
        }
    }
    sys_close(fd);

#ifdef DEBUG
    fd = sys_open("/prog_no_arg", O_RDONLY);
    memset(prog_buf, 0, file_size);
    sys_read(fd, prog_buf, 7);
    uint8_t* _tmp = prog_buf;
    printk("ELF Magic Number: ");
    for (int i = 0; i < 6; ++i) {
        printk("%x ", *_tmp);
        _tmp ++;
    }
    printk("\n");
    sys_close(fd);
#endif

    cls_screen();
    printf("[MAIN]: ENTER to get shell!\n");

    while(1);
    return 0;
}

void init(void)
{
    uint32_t ret_pid = fork();
    if (ret_pid) {
        // printf("[PID %d] I am parent, child's pid: %d\n", getpid(), ret_pid);
        while (1);
    } else {
        // printf("[PID %d] I am child, ret_pid: %d\n", getpid(), ret_pid);
        my_shell();
    }
    
    panic("init: should not be here");
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
