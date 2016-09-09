/*
 * MikeOS: Initial ramdisk(a very simple ram file system)
 */
#include "kmalloc.h"
#include "memory.h"
#include "list.h"
#include "vfs.h"
#include "string.h"

typedef struct initrd_inode
{
    uint32_t offset_from_start;
    uint32_t size;
    char name[20];
} __attribute__((packed)) initrd_inode_t;

static initrd_inode_t* search_inode_table(initrd_inode_t* inode_table_start,
                                          uint32_t number_of_inodes, char* name);

static initrd_inode_t* initrd_search_inode_table(initrd_inode_t* inode_table_start, uint32_t number_of_inodes, char* name)
{
    for(int i = 0; i < number_of_inodes; i++)
    {
        if(strcmp(inode_table_start[i].name, name) == 0)
            return &inode_table_start[i];

    }

    return NULL;
}

fs_node_t* initrd_open(file_system_t* fs, char* path)
{
    if(fs == NULL || path == NULL)
    {
        return NULL;
    }

    //There are no directories in initrd, lets just get the file.
    uint32_t* initrd_start_addr = (uint32_t*)fs->partition_begin_offset;

    uint32_t number_of_inodes = *((uint32_t*)initrd_start_addr);

    initrd_inode_t* requested_inode = initrd_search_inode_table((initrd_inode_t*)(initrd_start_addr + 1),
                                                                number_of_inodes,
                                                                path);

    //NO SUCH FILE..
    if(requested_inode == NULL)
    {
        return NULL;
    }

    fs_node_t* fs_node = kmalloc(sizeof(fs_node_t));
    if(fs_node == NULL)
    {
        //What, no more memory on kheap
        return NULL;
    }

    memcpy(fs_node->name, requested_inode->name, strlen(requested_inode->name) + 1);
    //The address of this shitty, shitty file.

    //TODO: Add kernel virtual offset... think of a nice API
    //For now its good because 4K of identity map
    char* inode_table_end_addr = ((char*)initrd_start_addr + sizeof(uint32_t) +
                                                                *initrd_start_addr * sizeof(initrd_inode_t));

    fs_node->fs_specific = (uint32_t)(inode_table_end_addr + (uint32_t)requested_inode->offset_from_start);

    fs_node->size = requested_inode->size;
    fs_node->fs = fs;

    return fs_node;
}

void initrd_close(fs_node_t* node)
{
    if(node != NULL)
        kfree(node);
}

uint32_t initrd_read(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer)
{
    if(node == NULL || offset < 0 || length <= 0 || buffer == NULL)
    {
        return 0;
    }

    if(node->size < offset + length)
    {
        return 0;
    }

    char* file_address = (char*)node->fs_specific;
    memcpy(buffer, file_address + offset, length);

    return length;
}
