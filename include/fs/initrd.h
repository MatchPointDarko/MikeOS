#ifndef MIKE_OS_INITRD_H
#define MIKE_OS_INITRD_H

#include <common.h>

typedef struct fs_node fs_node_t;
typedef struct file_system file_system_t;

uint32_t initrd_read(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer);
void initrd_close(fs_node_t* node);
fs_node_t* initrd_open(file_system_t* fs, const char* path);

#endif //MIKE_OS_INITRD_H
