#ifndef MIKE_OS_HARDDRIVE_H
#define MIKE_OS_HARDDRIVE_H

#include "common.h"

typedef struct ata_device ata_device_t;

void ata_init();
void ata_read(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count);
void ata_write(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count);

#endif //MIKE_OS_HARDDRIVE_H
