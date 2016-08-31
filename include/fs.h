#ifndef MIKE_OS_FS_H
#define MIKE_OS_FS_H

#include "common.h"
#include "device.h"

typedef struct file_system
{
    char name[20];
    block_device_t* device;
    uint32_t fs_specific;
    uint32_t partition_begin_offset;
    uint32_t partition_end_offset;

} file_system_t;

typedef struct file
{
    uint32_t flags;
    uint32_t mode;
    uint32_t offset;
    //inode_t* inode;
} file_t;

#endif //MIKE_OS_FS_H
