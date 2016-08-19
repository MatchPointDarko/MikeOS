#ifndef MIKE_OS_HARDDRIVE_H
#define MIKE_OS_HARDDRIVE_H

#include "common.h"

typedef struct ata_device ata_device_t;

/* Function: ata_init
 * ------------------
 * Initialize the ATA Driver.
 * */
void ata_init();

/* Function: ata_read
 * ------------------
 * Read sectors from an ATA device.
 *
 * device: the ata device to read from.
 * buffer: the buffer to read to.
 * sector: the starting sector.
 * sectors_count: the number of sectors to read.
 */
void ata_read(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count);

/* Function: ata_write
 * ------------------
 * Write sectors to an ATA device.
 *
 * device: the ata device to write to.
 * buffer: the buffer to read from.
 * sector: the starting sector.
 * sectors_count: number of sectors to write.
 */
void ata_write(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count);

#endif //MIKE_OS_HARDDRIVE_H
