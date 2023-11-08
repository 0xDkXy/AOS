#ifndef __FS_FILE_H
#define __FS_FILE_H

#include "inode.h"
#include "stdint.h"

struct file {
    uint32_t fd_pos;
    uint32_t fd_flag;
    struct inode* fd_inode;
};

enum std_fd {
    stdin_no,
    stdout_no,
    stderr_no
};

enum bitmap_type {
    INODE_BITMAP,
    BLOCK_BITMAP
};

#define MAX_FILE_OPEN 32

int32_t block_bitmap_alloc(struct partition* part);
void bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp);

#endif // __FS_FILE_H
