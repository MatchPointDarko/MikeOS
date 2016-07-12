/* Hard-drive driver, using LBA28, PIO, Dummy polling. */

#include "port_io.h"
#include "harddrive.h"

#define NULL 0x0
#define MAGIC_NUMBER(sector) ((0xE0) | (sector << 4) | ((sector >> 24) & 0x0F))

int has_harddrive()
{






}

static void pre_stages(int sector)
{
    write_port(0x1F1, NULL);
    write_port(0x1F2, 0x01);
    write_port(0x1F3, (unsigned char)sector);
    write_port(0x1F4, (unsigned char)(sector >> 8));
    write_port(0x1F5, (unsigned char)(sector >> 16));
    write_port(0x1F6, MAGIC_NUMBER(sector));
}

static void read_pre_stages(int sector)
{
    pre_stages(sector);
    write_port(0x1F7, 0x20);
}

static void write_pre_stages(int sector)
{
    pre_stages(sector);
    write_port(0x1F7, 0x30);
}

static inline void wait_for_drive_signal()
{
    while(!(read_port(0x1F7) & 0x08));
}

void read(hdd_block_t* chunk)
{
    read_pre_stages(1);
    wait_for_drive_signal();

    for(int i = 0; i < SECTOR_SIZE ; i++)
    {
        chunk->chunk[i] = read_port(0x1F0);
    }
}

void write(hdd_block_t* chunk)
{
    write_pre_stages(1);
    wait_for_drive_signal();

    for(int i = 0; i < SECTOR_SIZE; i++)
    {
        write_port(0x1F0, chunk->chunk[i]);
    }
}
