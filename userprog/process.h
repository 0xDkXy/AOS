#ifndef __USERPROG_PROCESS_H
#define __USERPROG_PROCESS_H

#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START    0x8048000
#define DEFAULT_PRIO    31

#include "thread.h"

void process_activate(struct task_struct* p_thread);
void process_execute(void* filename, char* name);
void page_dir_activate(struct task_struct* p_thread);
uint32_t* create_page_dir(void);

// debug
void print_cr3();


#endif // __USERPROG_PROCESS_H
