/* Hard-drive driver, using LBA28, PIO, Dummy polling.
 * TODO: Add usage of DMA in future versions, PIO Is good for now.
 */

#include "logger.h"
#include "common.h"
#include "port_io.h"
#include "harddrive.h"

#define MAGIC_NUMBER(sector) ((0xE0) | (sector << 4) | ((sector >> 24) & 0x0F))
#define HAS_HARDDRIVE (result & 0x40)
#define HD_INPUT_PORT (0x1F6)
#define HD_STATUS_PORT (0x1F7)

bool_t has_master_hd = false;

void harddrive_init()
{
    //Check for master hd
    write_port(HD_INPUT_PORT, 0xA0);

    unsigned int result = read_port(HD_STATUS_PORT);

    if(HAS_HARDDRIVE)
    {
        log_print(LOG_DEBUG, "Master harddrive exists, initialized successfuly!");
        has_master_hd = true;
    }
}

static void pre_stages(int sector)
{
    write_port(0x1F1, (unsigned int)NULL);
    write_port(0x1F2, 0x01);
    write_port(0x1F3, (unsigned char)sector);
    write_port(0x1F4, (unsigned char)(sector >> 8));
    write_port(0x1F5, (unsigned char)(sector >> 16));
    write_port(HD_INPUT_PORT, MAGIC_NUMBER(sector));
}

static void read_pre_stages(int sector)
{
    pre_stages(sector);
    write_port(HD_STATUS_PORT, 0x20);
}

static void write_pre_stages(int sector)
{
    pre_stages(sector);
    write_port(HD_STATUS_PORT, 0x30);
}

static inline void wait_for_drive_signal()
{
    while(!(read_port(HD_STATUS_PORT) & 0x08));
}

bool_t read(hdd_block_t* chunk)
{
    if(!has_master_hd)
    {
        return false;
    }

    read_pre_stages(1);
    wait_for_drive_signal();

    for(int i = 0; i < SECTOR_SIZE ; i++)
    {
        chunk->chunk[i] = read_port(0x1F0);
    }

    return true;
}

bool_t write(hdd_block_t* chunk)
{
    if(!has_master_hd)
    {
        return false;
    }

    write_pre_stages(1);
    wait_for_drive_signal();

    for(int i = 0; i < SECTOR_SIZE; i++)
    {
        write_port(0x1F0, chunk->chunk[i]);
    }

   return true;
}
