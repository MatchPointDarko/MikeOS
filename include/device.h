#ifndef MIKE_OS_DEVICE_H
#define MIKE_OS_DEVICE_H

#include "common.h"
#define MAX_DEVICES (50)
#define MAX_NAME_SIZE (41)

typedef struct block_device
{
    char name[MAX_NAME_SIZE];

    void (*write) (struct block_device* dev, void* buf, uint32_t begin_sector, uint32_t num_sectors);
    void (*read) (struct block_device* dev, void* buf, uint32_t begin_sector, uint32_t num_sectors);

    //Specific information for each device.
    uint32_t device_specific;

    //Used for drive devices.
    uint32_t device_size;
    uint32_t sector_size;

} block_device_t;

#endif //MIKE_OS_DEVICE_H
