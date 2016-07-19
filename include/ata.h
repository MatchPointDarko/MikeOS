#ifndef MIKE_OS_HARDDRIVE_H
#define MIKE_OS_HARDDRIVE_H
#include "common.h"

#define SECTOR_SIZE 512

typedef struct hdd_block
{
    char chunk[SECTOR_SIZE];
} hdd_block_t;

void harddrive_init();
bool_t write_sector(hdd_block_t* chunk, unsigned int sector);
bool_t read_sector(hdd_block_t* chunk, unsigned int sector);

#endif //MIKE_OS_HARDDRIVE_H
