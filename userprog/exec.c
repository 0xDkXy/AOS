#include "exec.h"
#include "global.h"
#include "stdint.h"
#include <stdbool.h>
#include "memory.h"
#include "fs.h"
#include "string.h"
#include "thread.h"
#include "stdio-kernel.h"

extern void intr_exit(void);
typedef uint32_t ELF32_Word, ELF32_Addr, ELF32_Off;
typedef uint16_t ELF32_Half;

struct ELF32_Ehdr {
    unsigned char   e_ident[16];
    ELF32_Half      e_type; 
    ELF32_Half      e_machine;
    ELF32_Word      e_version;
    ELF32_Addr      e_entry;
    ELF32_Off       e_phoff;
    ELF32_Off       e_shoff;
    ELF32_Word      e_flags;
    ELF32_Half      e_ehsize;
    ELF32_Half      e_phentsize;
    ELF32_Half      e_phnum;
    ELF32_Half      e_shentsize;
    ELF32_Half      e_shnum;
    ELF32_Half      e_shstrndx;
};

struct ELF32_Phdr {
    ELF32_Word  p_type;
    ELF32_Off   p_offset;
    ELF32_Addr  p_vaddr;
    ELF32_Addr  p_paddr;
    ELF32_Word  p_filesz;
    ELF32_Word  p_memsz;
    ELF32_Word  p_flags;
    ELF32_Word  p_align;
};

enum segment_type {
    PT_NULL,
    PT_LOAD,
    PT_DYNAMIC,
    PT_INTERP,
    PT_NOTE,
    PT_SHLIB,
    PT_PHDR
};

static bool segment_load(int32_t fd, uint32_t offset, uint32_t filesz, uint32_t vaddr)
{
    uint32_t vaddr_first_page = vaddr & 0xfffff000;
    uint32_t size_in_first_page = PG_SIZE - (vaddr & 0x00000fff);
    uint32_t occupy_pages = 0;

    if (filesz > size_in_first_page) {
        uint32_t left_size = filesz - size_in_first_page;
        occupy_pages = DIV_ROUND_UP(left_size, PG_SIZE) + 1;
    } else {
        occupy_pages = 1;
    }

    uint32_t page_idx = 0;
    uint32_t vaddr_page = vaddr_first_page;
    while (page_idx < occupy_pages) {
        uint32_t* pde = pde_ptr(vaddr_page);
        uint32_t* pte = pte_ptr(vaddr_page);

#ifdef BOCHS_DEBUG
printk("stop in FILE %s Line %d Func %s\n", __FILE__, __LINE__, __func__);
asm volatile ("xchgw %%bx, %%bx":::"memory");
#endif
        if (!(*pde & 0x00000001) || !(*pte & 0x00000001) ) {
            if (get_a_page(PF_USER, vaddr_page) == NULL) {
                return false;
            }
        }
        vaddr_page += PG_SIZE;
        page_idx ++;
    }
    sys_lseek(fd, offset, SEEK_SET);
    sys_read(fd, (void*)vaddr, filesz);
    return true;
}
    

static int32_t load(const char* pathname)
{
    int32_t ret = -1;
    struct ELF32_Ehdr elf_header;
    struct ELF32_Phdr prog_header;
    memset(&elf_header, 0, sizeof(struct ELF32_Ehdr));

    int32_t fd = sys_open(pathname, O_RDONLY);
    if (fd == -1)  {
        return -1;
    }

    if (sys_read(fd, &elf_header, sizeof(struct ELF32_Ehdr)) != sizeof(struct ELF32_Ehdr)) {
        ret = -1;
        goto done;
    }

    if (memcmp(elf_header.e_ident, "\177ELF\1\1\1", 7) ||
            elf_header.e_type != 2 ||
            elf_header.e_machine != 3 ||
            elf_header.e_version != 1 ||
            elf_header.e_phnum > 1024 ||
            elf_header.e_phentsize != sizeof(struct ELF32_Phdr)) {
#ifdef DEBUG
    printk("ELF header broken!\n");
    uint8_t* identy = &elf_header.e_ident;
    printk("e_ident: ");
    for (int i = 0; i < 6; ++i) {
        printk("%x ", *identy);
        identy++;
    }
    printk("\ne_type: %d\n", elf_header.e_type);
    printk("e_machine: %d\n", elf_header.e_machine);
    printk("e_version: %d\n", elf_header.e_version);
    printk("e_phnum: %d\n", elf_header.e_phnum);
    printk("e_phentsize: %d\n", elf_header.e_phentsize);
#endif
        ret = -1;
        goto done;
    }

    ELF32_Off prog_header_offset = elf_header.e_phoff;
    ELF32_Half prog_header_size = elf_header.e_phentsize;

    uint32_t prog_idx = 0;
    while (prog_idx < elf_header.e_phnum) {
        memset(&prog_header, 0, prog_header_size);

        sys_lseek(fd, prog_header_offset, SEEK_SET);

        if (sys_read(fd, &prog_header, prog_header_size) != prog_header_size) {
            ret = -1;
            goto done;
        }

        if (prog_header.p_type == PT_LOAD) {
            if (!segment_load(fd, prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr)) {
                ret = -1;
                goto done;
            }
        }

        prog_header_offset += elf_header.e_phentsize;
        prog_idx ++;
    }
    ret = elf_header.e_entry;

done:
    sys_close(fd);
    return ret;
}

int32_t sys_execv(const char* path, const char* argv[])
{
    uint32_t argc = 0;
    while (argv[argc]) {
        argc++;
    }
    int32_t entry_point = load(path);
    if (entry_point == -1) {
        return -1;
    }

    struct task_struct* cur = running_thread();

    memcpy(cur->name, path, TASK_NAME_LEN);
    cur->name[TASK_NAME_LEN - 1] = 0;

    struct intr_stack* intr_0_stack = (struct intr_stack*)((uint32_t)cur + PG_SIZE - sizeof(struct intr_stack));
    intr_0_stack->ebx = (int32_t) argv;
    intr_0_stack->ecx = argc;
    intr_0_stack->eip = (void*)entry_point;
    intr_0_stack->esp = (void*)0xc0000000;

    asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g"(intr_0_stack) : "memory");
    return 0;
}
