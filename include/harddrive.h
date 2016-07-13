#ifndef MIKE_OS_HARDDRIVE_H
#define MIKE_OS_HARDDRIVE_H
#include "common.h"

#define SECTOR_SIZE 512

typedef struct hdd_block
{
    char chunk[SECTOR_SIZE];
} hdd_block_t;

void harddrive_init();
bool_t write(hdd_block_t* chunk);
bool_t read(hdd_block_t* chunk);

#endif //MIKE_OS_HARDDRIVE_H
