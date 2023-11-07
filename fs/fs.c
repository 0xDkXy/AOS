#include "fs.h"
#include "global.h"
#include "ide.h"
#include "kernel/list.h"
#include "memory.h"
#include "stdint.h"
#include "inode.h"
#include "stdio-kernel.h"
#include "dir.h"
#include "super_block.h"
#include "ide.h"
#include "string.h"
#include "debug.h"

struct partition* cur_part; // default partition

static bool mount_partition(struct list_elem* pelem, int arg)
{
    char* part_name = (char*)arg;
    struct partition* part = elem2entry(struct partition, part_tag, pelem);
    if (!strcmp(part->name, part_name)) {
        cur_part = part;
        struct disk* hd = cur_part->my_disk;

        struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);

        cur_part->sb = (struct super_block*)sys_malloc(sizeof(struct super_block));
        if (cur_part->sb == NULL) {
            PANIC("alloc memory failed!");
        }

        memset(sb_buf, 0, SECTOR_SIZE);
        ide_read(hd, cur_part->start_lba + 1, sb_buf, 1);

        memcpy(cur_part->sb, sb_buf, sizeof(struct super_block));
        cur_part->block_bitmap.bits = 
            (uint8_t*)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);

        if (cur_part->block_bitmap.bits == NULL) {
            PANIC("alloc memory failed!");
        }
        cur_part->block_bitmap.btmp_bytes_len = sb_buf->block_bitmap_sects * SECTOR_SIZE;

        ide_read(hd, sb_buf->block_bitmap_lba, cur_part->block_bitmap.bits, sb_buf->block_bitmap_sects);
        cur_part->inode_bitmap.bits = (uint8_t*)sys_malloc(sb_buf->inode_bitmap_sects * SECTOR_SIZE);
        if (cur_part->inode_bitmap.bits == NULL) {
            PANIC("alloc memory failed!");
        }
        cur_part->inode_bitmap.btmp_bytes_len = sb_buf->inode_bitmap_sects * SECTOR_SIZE;

        ide_read(hd, sb_buf->inode_bitmap_lba, cur_part->inode_bitmap.bits, sb_buf->inode_bitmap_sects);
        list_init(&cur_part->open_inodes);
        printk("mount %s done!\n", part->name);

        return true;
    }
    return false;
}

static void partition_format(struct partition* part)
{

    uint32_t boot_sector_sects = 1;
    uint32_t super_block_sects = 1;
    uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);
    uint32_t inode_table_sects = 
        DIV_ROUND_UP((sizeof(struct inode) * MAX_FILES_PER_PART), SECTOR_SIZE);
    uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;

    uint32_t block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);

    struct super_block sb;
    sb.magic = 19260817;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba = sb.part_lba_base + 2;
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no = 0;
    sb.dir_entry_size = sizeof(struct dir_entry);

    printk("%s info:\n", part->name);
    printk("    magic:                  0x%x\n", sb.magic);
    printk("    part_lba_base:          0x%x\n", sb.part_lba_base);
    printk("    all_sectors:            0x%x\n", sb.sec_cnt);
    printk("    inode_cnt:              0x%x\n", sb.inode_cnt);
    printk("    block_bitmap_lba:       0x%x\n", sb.block_bitmap_lba);
    printk("    block_bitmap_sectors:   0x%x\n", sb.block_bitmap_sects);
    printk("    inode_bitmap_lba:       0x%x\n", sb.inode_bitmap_lba);
    printk("    inode_bitmap_sectors:   0x%x\n", sb.inode_bitmap_sects);
    printk("    inode_table_lba:        0x%x\n", sb.inode_table_lba);
    printk("    inode_table_sectors:    0x%x\n", sb.inode_table_sects);
    printk("    data_start_lba:         0x%x\n", sb.data_start_lba);

    struct disk* hd = part->my_disk;

    /* write super block into sector 1 */
    ide_write(hd, part->start_lba + 1, &sb, 1);
    printk("    super_block_lba: 0x%x\n", part->start_lba + 1);

    uint32_t buf_size =
        sb.block_bitmap_sects >= sb.inode_bitmap_sects ? 
        sb.block_bitmap_sects : sb.inode_bitmap_sects;
    buf_size = 
        buf_size > sb.inode_table_sects ?
        buf_size : sb.inode_table_sects;
    buf_size *= SECTOR_SIZE;

    uint8_t* buf = (uint8_t*)sys_malloc(buf_size);

    /* initialize block bitmap and write it into super block*/
    buf[0] |= 0x01;
    uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
    uint8_t block_bitmap_last_bit = block_bitmap_bit_len % 8;
    uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);

    memset(&buf[block_bitmap_last_byte], 0xff, last_size);

    uint8_t bit_idx = 0;
    while (bit_idx <= block_bitmap_last_bit) {
        buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
    }
    ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

    /* initialize inode bitmap and write it into super block*/

    memset(buf, 0, buf_size);
    buf[0] |= 0x1;
    ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

    /* initialize inode array and write it into super block inode_table_lba */
    memset(buf, 0, buf_size);
    struct inode* i = (struct inode*)buf;
    i->i_size = sb.dir_entry_size * 2;  // for . and .. dir
    i->i_no = 0;    // root
    i->i_sectors[0] = sb.data_start_lba;
    ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

    /* write root into sb.data_start_lba */
    memset(buf, 0, buf_size);
    struct dir_entry* p_de = (struct dir_entry*)buf;

    memcpy(p_de->filename, ".", 1);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;

    memcpy(p_de->filename, "..", 2);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;

    ide_write(hd, sb.data_start_lba, buf, 1);

    printk("    root_dir_lba: 0x%x\n", sb.data_start_lba);
    printk("%s format done\n", part->name);
    sys_free(buf);

}

void filesys_init()
{
    uint8_t channel_no = 0, dev_no, part_idx = 0;

    /* there is a bug. If we allocate memory to sb_buf directly, the arena
     * information will be cleaned after the line calling ide_read function.
     * Specifically, in ide_read function, the main thread yield and then the
     * idle thread will read from disk then give the control back to main
     * thread. But in this process, in the process of switching thread, there
     * will be a function to activate the page dir of process. Even though they
     * are two threads, not processes, the function still need to move the same
     * page dir address to the register CR3. The problem occurs here. After
     * moving the kernel page dir address 0x10000(physical address) to CR3, the
     * arena information of `sb_buf` suddenly disappears, which will occur page
     * fault in subsequent `sys_free`. In order to avoid this, we first allocate
     * a full-page memory to `_temp`, so the `sb_buf` will allocated to the
     * second page in memory pool. It's just a temporary way to avoid this bug.
     * But for the subsequent developing, I have to do it because I have already
     * spend almost one week on fixing it.
     *
     * I will fix this bug later.
     */
    struct super_block* _temp = (struct super_block*)sys_malloc(1024 * 4);
    struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE * 2);
    sys_free(_temp);

    if (sb_buf == NULL) {
        PANIC("alloc memory failed!");
    }
    printk("searching filesystem......\n");
    while (channel_no < channel_cnt) {
        dev_no = 0;
        while (dev_no < 2) {
            if (dev_no == 0) {
                dev_no ++;
                continue;
            }
            struct disk* hd = &channels[channel_no].devices[dev_no];
            struct partition* part = hd->prim_parts;
            while (part_idx < 12) {
                if (part_idx == 4) {
                    part = hd->logic_parts;
                }

                if (part->sec_cnt != 0) {
                    memset(sb_buf, 0, SECTOR_SIZE);

                    // check_arena((void*)sb_buf);
                    // BUG
                    ide_read(hd, part->start_lba + 1, sb_buf, 1);

                    // check_arena((void*)sb_buf);
                    if (sb_buf->magic == 19260817) {
                        printk("%s has filesystem\n", part->name);
                    } else {
                        printk("formatting %s's partition %s......\n", hd->name, part->name);
                        partition_format(part);
                    }
                }
                part_idx ++;
                part ++;
            }
            dev_no ++;
        }
        channel_no ++;
    }
    sys_free(sb_buf);

    char default_part[8] = "sdb1";
    list_traversal(&partition_list, mount_partition, (int)default_part);

}