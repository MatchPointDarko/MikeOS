#ifndef MIKE_OS_HARDDRIVE_H
#define MIKE_OS_HARDDRIVE_H

#include "common.h"
#include "device.h"

typedef struct ata_device ata_device_t;

/* Function: ata_init
 * ------------------
 * Initialize the ATA Driver.
 * */
void ata_init();

/* Function: read
 * ------------------
 * Read a buffer from an ATA Drive.
 *
 * dev: the device to read from.
 * buf: the buffer to write to.
 * offset: the offset to read from.
 * size: size to read.
 */
void ata_read(block_device_t* dev, void* buf, uint32_t offset, uint32_t size);

/* Function: write
 * ------------------
 * write a buffer to an ATA Drive.
 *
 * dev: the device to write to.
 * buf: the buffer to read from.
 * offset: the offset to write from.
 * size: size to write.
 */
void ata_write(block_device_t* dev, void* buf, uint32_t offset, uint32_t size);

#endif //MIKE_OS_HARDDRIVE_H
