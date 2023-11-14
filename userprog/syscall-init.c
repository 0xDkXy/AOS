#include "syscall-init.h"
#include "user/syscall.h"
#include "stdint.h"
#include "thread.h"
#include "kernel/print.h"
#include "console.h"
#include "string.h"
#include "fs.h"
#include "fork.h"

#define syscall_nr 32
typedef void* syscall;
syscall syscall_table[syscall_nr];

uint32_t sys_getpid(void)
{
    return running_thread()->pid;
}

void syscall_init(void)
{
    put_str("syscall_init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_MALLOC] = sys_malloc;
    syscall_table[SYS_FREE] = sys_free;
    syscall_table[SYS_FORK] = sys_fork;
    put_str("syscall_init done\n");
}
