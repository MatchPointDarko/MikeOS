/* Hard-drive ATA driver, using LBA28, PIO, Dummy polling.
 * TODO: Add usage of DMA in future versions, Polling is good for now.
 */
#if 0
#include "logger.h"
#include "common.h"
#include "port_io.h"
#include "ata.h"

#define M_COMMAND_REGISTER (0x1F7)
#define M_COMMAND_REGISTER2 (0x3F6)
#define M_ERROR_REGISTER (0x1F1)
#define M_STATUS_REGISTER (0x1F7)
#define M_DATA_REGISTER (0x1F0)
#define M_BASE (0x1F0)
#define M_DRIVE_REGISTER (0x1F6)

#define S_COMMAND_REGISTER (0x1F7)
#define S_COMMAND_REGISTER2 (0x3F6)
#define S_ERROR_REGISTER (0x1F1)
#define S_STATUS_REGISTER (0x1F7)
#define S_DATA_REGISTER (0x1F0)
#define S_BASE (0x1F0)

typedef enum device_status
{
    DEVICE_BUSY = 0x80,
    DEVICE_READY = 0x40,
    DEVICE_FAULT = 0x20,
    DEVICE_SEEK_COMPLETE = 0x10,
    DEVICE_DATA_READY = 0x08,
    DEVICE_ERROR = 0x01

} device_status_t;

typedef enum ata_error
{
    BAD_BLOCK = 0x80,
    UNCORRECTABLE_DATA = 0x40,
    NO_MEDIA = 0x20,
    MARK_NOT_FOUND = 0x10,
    COMMAND_ABORT = 0x04,
    ADDRESS_MARK_NOT_FOUND = 0x01
} ata_error_t;

typedef enum ata_command
{
    ATA_CMD_READ_PIO = 0x20,
    ATA_CMD_READ_PIO_EXT = 0x24,
    ATA_CMD_READ_DMA = 0xC8,
    ATA_CMD_READ_DMA_EXT = 0x25,
    ATA_CMD_WRITE_PIO = 0x30,
    ATA_CMD_WRITE_PIO_EXT = 0x34,
    ATA_CMD_WRITE_DMA = 0xCA,
    ATA_CMD_WRITE_DMA_EXT = 0x35,
    ATA_CMD_CACHE_FLUSH = 0xE7,
    ATA_CMD_CACHE_FLUSH_EXT = 0xEA,
    ATA_CMD_PACKET = 0xA0,
    ATA_CMD_IDENTIFY_PACKET = 0xA1,
    ATA_CMD_IDENTIFY = 0xEC
} ata_command_t;

typedef enum atapi_command
{
    ATAPI_CMD_READ = 0xA8,
    ATAPI_CMD_EJECT = 0x1B
} atapi_command_t;

typedef enum device_type
{
    ATA,
    ATAPI
} device_type_t;


#define MAGIC_NUMBER(sector) ((0xE0) | (sector << 4) | ((sector >> 24) & 0x0F))
#define HAS_HARDDRIVE(result) (result & 0x40)
#define HD_INPUT_PORT (0x1F6)
#define HD_STATUS_PORT (0x1F7)
#define HD_READ_PORT (0x1F0)
#define HD_WRITE_PORT (0x1F0)

bool_t has_master_ata = false;

typedef struct ata_device
{
    unsigned long base;
    bool_t exists;
    bool_t is_primary_channel;
    bool_t is_master;
    device_type_t device_type;
    unsigned short signature;
    unsigned short featuers;
    unsigned int command_supported;
    unsigned long sector size;
    unsigned char model[41];
} ata_device_t;

//Total ATA Devices.
ata_device_t devices[4];

void ata_init()
{
    //Check for master hd
    write_port(M_DRIVE_REGISTER, 0xA0);

    unsigned int result = read_port(M_DRIVE_REGISTER);

    if(HAS_HARDDRIVE(result))
    {
        log_print(LOG_DEBUG, "Master ATA exists");
        master.exists = true;
    }
}

void ata_wait(device_status_t status)
{



}

static void pre_stages(unsigned int sector)
{
    write_port(0x1F1, (unsigned int)NULL);
    write_port(0x1F2, 0x01);
    write_port(0x1F3, (unsigned char)sector);
    write_port(0x1F4, (unsigned char)(sector >> 8));
    write_port(0x1F5, (unsigned char)(sector >> 16));
    write_port(HD_INPUT_PORT, MAGIC_NUMBER(sector));
}

static void read_pre_stages(unsigned int sector)
{
    pre_stages(sector);
    write_port(HD_STATUS_PORT, 0x20);
}

static void write_pre_stages(unsigned int sector)
{
    pre_stages(sector);
    write_port(HD_STATUS_PORT, 0x30);
}

static inline void wait_for_drive_signal()
{
    while(!(read_port(HD_STATUS_PORT) & 0x08));
}

bool_t read_sector(hdd_block_t* chunk, unsigned int sector)
{

    read_pre_stages(sector);
    wait_for_drive_signal();

    for(int i = 0; i < SECTOR_SIZE ; i++)
    {
        chunk->chunk[i] = read_word_port(HD_READ_PORT);
    }

    return true;
}

bool_t write_sector(hdd_block_t* chunk, unsigned int sector)
{
    write_pre_stages(sector);
    wait_for_drive_signal();

    for(int i = 0; i < SECTOR_SIZE; i++)
    {
        write_port(HD_WRITE_PORT, chunk->chunk[i]);
    }

    return true;
}
#endif
