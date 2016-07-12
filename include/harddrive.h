#ifndef MIKE_OS_HARDDRIVE_H
#define MIKE_OS_HARDDRIVE_H

#define SECTOR_SIZE 512

typedef struct hdd_block
{
    char chunk[SECTOR_SIZE];
} hdd_block_t;

void write(hdd_block_t* chunk);
void read(hdd_block_t* chunk);

#endif //MIKE_OS_HARDDRIVE_H
