/*
 * MikeOS: ATA Driver. PIO, LBA28 is good for now.
 * TODO: Add usage of DMA, LBA48, ATAPI, in the future.
 */
#include "logger.h"
#include "common.h"
#include "port_io.h"
#include "panic.h"
#include "ata.h"

#define ATA_DATA_PORT (0x0)
#define ATA_FEATURES_PORT (0x1)
#define ATA_SECTOR_COUNT_PORT (0x2)
#define ATA_LBA_LOW_PORT (0x3)
#define ATA_LBA_MID_PORT (0x4)
#define ATA_LBA_HIGH_PORT (0x5)
#define ATA_DRIVE_PORT (0x6)
#define ATA_COMMAND_PORT (0x7)
#define ATA_STATUS_PORT (0x7)
#define ALTERNATIVE_CONTROL_PORT (0x206)

#define FIRST_ENTRY_MASTER (0)
#define FIRST_ENTRY_SLAVE (1)

#define SECOND_ENTRY_MASTER (2)
#define SECOND_ENTRY_SLAVE (3)

#define MAX_ATA_DRIVES (4)
#define FIRST_ENTRY (0x0)
#define SECOND_ENTRY (0x2)

#define DEFAULT_SECTOR_SIZE (512)
//256 words, i.e. 512 bytes.
#define IDENTIFY_BUFFER_SIZE (256)

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
    ATAPI,
    SATA
} device_type_t;


#define MAGIC_NUMBER(sector) ((0xE0) | (sector << 4) | ((sector >> 24) & 0x0F))
#define HAS_HARDDRIVE(result) (result & 0x40)
#define HD_INPUT_PORT (0x1F6)
#define HD_STATUS_PORT (0x1F7)
#define HD_READ_PORT (0x1F0)
#define HD_WRITE_PORT (0x1F0)

typedef struct ata_device
{
    unsigned long base;
    device_type_t device_type;
    unsigned short signature;
    unsigned short features;
    unsigned int command_supported;
    unsigned long sector_size;
    unsigned long drive_size;
    unsigned char model[41];
    bool_t exists;
    bool_t is_lba48;
    bool_t is_slave;
} ata_device_t;

//Total ATA Devices.
ata_device_t ata_devices[MAX_ATA_DRIVES];

//block_device_t ...

//Sleep for 400ms.
static void ata_sleep(ata_device_t* device)
{
    if(device == NULL)
    {
        kernel_panic();
    }

    read_port(device->base + ALTERNATIVE_CONTROL_PORT);
    read_port(device->base + ALTERNATIVE_CONTROL_PORT);
    read_port(device->base + ALTERNATIVE_CONTROL_PORT);
    read_port(device->base + ALTERNATIVE_CONTROL_PORT);
}

static void ata_select_device(ata_device_t* device)
{
    if(device == NULL)
    {
        kernel_panic();
    }

    //Add six to the selection register port.
    write_port(device->base + ATA_DRIVE_PORT,
               device->is_slave ? 0xB0 : 0xA0);

    ata_sleep(device);
}

/*
 * Returns true if the identify was successful,
 * false if the device doesn't exist.
 */
static bool_t ata_identify(bool_t is_atapi, ata_device_t* device)
{
    if (device == NULL)
    {
        kernel_panic();
    }

    write_port(device->base + ATA_SECTOR_COUNT_PORT, 0);
    write_port(device->base + ATA_LBA_LOW_PORT, 0);
    write_port(device->base + ATA_LBA_MID_PORT, 0);
    write_port(device->base + ATA_LBA_HIGH_PORT, 0);

    ata_sleep(device);

    if(is_atapi)
    {
        write_port(device->base + ATA_COMMAND_PORT,
                   ATA_CMD_IDENTIFY_PACKET);
    }

    else
    {
         write_port(device->base + ATA_COMMAND_PORT,
                   ATA_CMD_IDENTIFY);
    }

    ata_sleep(device);

    return (bool_t)read_port(device->base + ATA_STATUS_PORT);
}

//Poll the status register for 4 times.
static device_status_t ata_poll(ata_device_t* device)
{
    unsigned char status = 0;

    for(int i = 0; i < 4; i++)
        status = read_port(device->base + ATA_STATUS_PORT);

    return status;
}

static bool_t extract_identify_buffer(ata_device_t* device,
                             uint_16 identify_buffer[IDENTIFY_BUFFER_SIZE])
{
    if(identify_buffer[0] == 0)
    {
        //This is not a hard drive.
        device->exists = false;
        return false;
    }

    device->signature = identify_buffer[0];
    device->features = identify_buffer[0x31];
    device->command_supported = identify_buffer[0x52] | (uint_32)identify_buffer[0x53] << 16;
    //Get LBA
    if(device->command_supported & (1 << 26))
    {
        //TODO: This shit doesn't even make sense, fix it later.
#if 0
        //LBA48
        device->is_lba48 = true;
        device->drive_size = identify_buffer[0x64];
        device->drive_size |= identify_buffer[0x64] << 16;
        device->drive_size = identify_buffer[0x64] << 32;
        device->drive_size = identify_buffer[0x64] << 48;
#endif
    }
    else
    {
        //LBA24
        device->is_lba48 = false;
        device->drive_size = identify_buffer[0x3c];
        device->drive_size = identify_buffer[0x3d] << 16;
    }

    //TODO:Get Model string

    return true;
}

void ata_init()
{
    device_status_t status = DEVICE_READY;
    uint_16 identify_buffer[IDENTIFY_BUFFER_SIZE] = {0,};

    ata_devices[FIRST_ENTRY].base = 0x1F0;
    ata_devices[SECOND_ENTRY].base = 0x170;

    ata_devices[FIRST_ENTRY_MASTER].base = ata_devices[FIRST_ENTRY].base;
    ata_devices[FIRST_ENTRY_SLAVE].base = ata_devices[FIRST_ENTRY].base;

    ata_devices[SECOND_ENTRY_MASTER].base = ata_devices[SECOND_ENTRY].base;
    ata_devices[SECOND_ENTRY_SLAVE].base = ata_devices[SECOND_ENTRY].base;

    //TODO:IRQ

    for(int i = 0; i < MAX_ATA_DRIVES; i++)
    {
        ata_device_t* device = &ata_devices[i];
        device->is_slave = i % 2;

        ata_select_device(device);

        if(!ata_identify(false, device))
        {
            continue;
        }

        device->exists = true;

        //Identify the type.
        unsigned char type1 = read_port(device->base + ATA_LBA_MID_PORT);
        unsigned char type2 = read_port(device->base + ATA_LBA_HIGH_PORT);

        if(type1 == 0x14 && type2 == 0x15)
        {
            //Use atapi identifiy.
            ata_identify(true, device);
            device->device_type = ATAPI;
            //TODO:device->sector_size = 0;
        }
        else if(type1 == 0x3c && type2 == 0xc3)
        {
            //SATA
            device->device_type = SATA;
        }
        else
        {
            device->device_type = ATA;
            //Probably a normal ATA device, no packets.
            device->sector_size = DEFAULT_SECTOR_SIZE;
        }

        //Read identifiy buffer response.
        for(int i = 0; i < IDENTIFY_BUFFER_SIZE; i++)
        {
            identify_buffer[i] = read_word_port(device->base + ATA_DATA_PORT);
        }

        if(!extract_identify_buffer(device, identify_buffer))
        {
            continue;
        }

        log_print(LOG_INFO, "Found a drive with type: %s, %s",
                  device->device_type == ATA ? "ATA" :
                  device->device_type == ATAPI ? "ATAPI" : "SATA",
                  device->is_slave ? "slave" : "master");
    }
}

static void ata_pio28_pre_stages(ata_device_t* device, uint_32 sector, uint_8 sector_count)
{
    write_port(device->base + ATA_DRIVE_PORT,
               device->is_slave? 0xF0 : 0xE0);

    write_port(device->base + ATA_SECTOR_COUNT_PORT,
               sector_count);

    write_port(device->base + ATA_LBA_LOW_PORT,
               (unsigned char)sector);
    write_port(device->base + ATA_LBA_MID_PORT,
               (unsigned char)(sector >> 8));
    write_port(device->base + ATA_LBA_HIGH_PORT,
               (unsigned char)(sector >> 16));
}

static void ata_pio28_read(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count)
{
    ata_pio28_pre_stages(device, sector, sector_count);

    write_port(device->base + ATA_COMMAND_PORT,
               ATA_CMD_READ_PIO);

    for(int i = 0; i < sector_count; i++)
    {
        for (int j = 0; j < DEFAULT_SECTOR_SIZE / 2; j++)
        {
            buffer[j + (DEFAULT_SECTOR_SIZE / 2) * i] = read_word_port(device->base + ATA_DATA_PORT);
        }
        ata_sleep(device);
    }
}

static void ata_pio28_write(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count)
{
    ata_pio28_pre_stages(device, sector, sector_count);

    write_port(device->base + ATA_COMMAND_PORT,
               ATA_CMD_WRITE_PIO);

    for(int i = 0; i < sector_count; i++)
    {
        for (int j = 0; j < DEFAULT_SECTOR_SIZE / 2; j++)
        {
            write_word_port(device->base + ATA_DATA_PORT, buffer[j + (DEFAULT_SECTOR_SIZE / 2) * i]);
            ata_sleep(device);
        }

        ata_sleep(device);
    }
}

void ata_read(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count)
{
    if(device == NULL || !device->exists || buffer == NULL || sector < 0 || sector_count < 0)
    {
        //TODO: Think if we actually need a panic here..
        kernel_panic();
    }

    ata_pio28_read(device, buffer, sector, sector_count);
}

void ata_write(ata_device_t* device, uint_16 buffer[], uint_32 sector, uint_8 sector_count)
{
    if(device == NULL || !device->exists || buffer == NULL || sector < 0 || sector_count < 0)
    {
        //TODO: Think if we actually need a panic here..
        kernel_panic();
    }

    ata_pio28_write(device, buffer, sector, sector_count);
}
