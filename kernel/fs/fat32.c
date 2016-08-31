#include "logger.h"
#include "common.h"
#include "fs.h"

typedef struct fat_boot_sector
{
    uint8_t bootjmp[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t	sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t	table_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t table_size_16;
    uint16_t sectors_per_track;
    uint16_t head_side_count;
    uint32_t hidden_sector_count;
    uint32_t total_sectors_32;

    //this will be cast to it's specific type once the driver actually knows what type of FAT this is.
    uint8_t extended_section[54];
} __attribute__((packed)) fat_boot_sector_t;

typedef struct fat_extBS_32
{
    //extended fat32 stuff
    uint32_t table_size_32;
    uint16_t extended_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fat_info;
    uint16_t backup_BS_sector;
    uint8_t reserved_0[12];
    uint8_t drive_number;
    uint8_t reserved_1;
    uint8_t	boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];

}__attribute__((packed)) fat_extBS_32_t;

void fat32_format(file_system_t* fs)
{
    if (fs == NULL || fs->partition_begin_offset >= fs->partition_end_offset)
    {
        return;
    }

    //Formatting FAT32 in offset 'fs->partition'
    log_print(LOG_INFO, "Creating a FAT32 partition in blk device: %s, between partitions %d-%d",
              fs->device->name, fs->partition_begin_offset, fs->partition_end_offset);

    uint8_t sector[fs->device->sector_size] = {0};

    fat_boot_sector_t *bs = (fat_boot_sector_t *) sector;

    bs->bootjmp[0] = 0xEB;
    bs->bootjmp[0] = 0x3C;
    bs->bootjmp[0] = 0x90;

    bs->bytes_per_sector = fs->device->sector_size;
    bs->reserved_sector_count = 2;
    bs->table_count = 2;
    bs->root_entry_count = 1;

    bs->total_sectors_32 = fs->device->device_size / fs->device->sector_size;
    bs->media_type = 0xF0;

    bs->sectors_per_track = 0;// We don't give a shit..
    bs->sectors_per_cluster = 1;
    bs->hidden_sector_count = fs->partition_begin_offset;

    fat_extBS_32_t *ext_bs = (fat_extBS_32_t *) bs->extended_section;
    ext_bs->table_size_32 = bs->total_sectors_32 - bs->hidden_sector_count;
    ext_bs->flags = 0; //...
    ext_bs->fat_version = 32;
    ext_bs->root_cluster = 3;
    ext_bs->boot_signature = 0x28;

    memcpy(ext_bs->volume_label, "           ", sizeof(ext_bs->volume_label));
    memcpy(ext_bs->fat_type_label, "FAT32 ", sizeof("FAT32 "));

    //Write the boot sector
    fs->device->write(fs->device, sector, fs->partition_begin_offset, 1);
}

static void read_cluster(file_system_t* fs, uint32_t cluster_index)
{
}

static void write_cluster(file_system_t* fs, uint32_t cluster_index, void* data)
{
}

static read_inode(const char* path)
{}

static alloc_inode(const char* path)
{}
