#ifndef __IDE_H
#define __IDE_H

#include "stdint.h"
#include "kernel/list.h"
#include "kernel/bitmap.h"
#include "thread.h"
#include "sync.h"

struct partition {
    uint32_t start_lba;
    uint32_t sec_cnt;
    struct disk* my_disk;
    struct list_elem part_tag;
    char name[8];
    struct super_block* sb;
    struct bitmap block_bitmap;
    struct bitmap inode_bitmap;
    struct list open_inodes;
};

struct disk {
    char name[8];
    struct ide_channel* my_channel;
    uint8_t dev_no;
    struct partition prim_parts[4];
    struct partition logic_parts[8];
};

struct ide_channel {
    char name[8];
    uint16_t port_base;
    uint8_t irq_no;
    struct lock lock;
    bool expecting_intr;
    struct semaphore disk_done;
    struct disk devices[2];
};

extern uint8_t channel_cnt;
extern struct ide_channel channels[2];
extern struct list partition_list;

// function declare
void ide_init();
void ide_write(struct disk*, uint32_t , void* , uint32_t );
void ide_read(struct disk*, uint32_t, void*, uint32_t);

#endif // __IDE_H
