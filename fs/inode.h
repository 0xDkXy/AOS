#ifndef __FS_INODE_H
#define __FS_INODE_H

#include "stdint.h"
#include <stdbool.h>
#include "kernel/list.h"

struct inode {
    uint32_t i_no; // inode number;
    uint32_t i_size;

    uint32_t i_open_cnts; // the times that it be opened;
    bool write_deny;    // write cannot be parallel

    uint32_t i_sectors[13];
    struct list_elem inode_tag;
};


struct inode* inode_open(struct partition* part, uint32_t inode_no);
void inode_close(struct inode* inode);
void inode_init(uint32_t inode_no, struct inode* new_inode);
void inode_sync(struct partition* part, struct inode* inode, void* io_buf);
void inode_release(struct partition* part, uint32_t inode_no);

#endif // __FS_INODE_H
