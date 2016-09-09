/*
 * MikeOS: FAT32(With some minor changes) Filesystem implementation.
 */
#if 0
#include "string.h"
#include "memory.h"
#include "logger.h"
#include "common.h"
#include "kmalloc.h"
#include "vfs.h"

#define IS_DIRECTORY(value) ((value) & 0x10)
#define FAT_EOC (0xFFFFFFFF)

typedef enum status
{ ERROR, SUCCESS, INVALID_ARGUMENT, NO_SUCH_FILE, INVALID_PATH} status_t;

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

typedef struct fat_file_entry
{
    char file_name[8];
    char file_extension[3];
    uint8_t file_attributes;
    uint8_t reserved[10];
    uint16_t time_created;
    uint16_t last_updated;
    uint16_t starting_cluster;
    uint32_t file_size;

} __attribute__((packed)) fat_file_entry_t;

typedef struct fat_directory_entry
{
    fat_file_entry_t *entries;
} fat_directory_entry_t;

void fat32_format(file_system_t* fs)
{
    if (fs == NULL || fs->partition_begin_offset >= fs->partition_end_offset)
    {
        return;
    }

    //Formatting FAT32 in offset 'fs->partition'
    log_print(LOG_INFO, "Creating a FAT32 partition in blk device: %s, between partitions %d-%d",
              fs->device->name, fs->partition_begin_offset, fs->partition_end_offset);

    uint8_t sector[fs->device->sector_size];

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
    ext_bs->extended_flags = 0; //...
    ext_bs->fat_version = 32;
    ext_bs->root_cluster = 2;
    ext_bs->boot_signature = 0x28;

    memcpy(ext_bs->volume_label, "           ", sizeof(ext_bs->volume_label));
    memcpy(ext_bs->fat_type_label, "FAT32 ", sizeof("FAT32 "));

    //Write the boot sector
    fs->device->write(fs->device, sector, fs->partition_begin_offset, 1);

    uint8_t shit[512] = {0};
    fat_file_entry_t* entries = shit;

    memcpy(entries[0].file_name, "shit", sizeof("shit"));
    entries[0].file_attributes = 0x10;
    entries[0].file_size = 20;

    memcpy(entries[1].file_name, "fuck", sizeof("shit"));
    entries[1].file_attributes = 0x10;
    entries[1].file_size = 20;

    memcpy(entries[2].file_name, "shit2", sizeof("shit2"));
    entries[2].file_attributes = 0x10;
    entries[2].file_size = 20;

    fs->device->write(fs->device, shit, fs->partition_begin_offset+2, 1);
}

inline void read_cluster(file_system_t* fs, uint32_t index, void* buf)
{
    uint8_t boot_sector[fs->device->sector_size];
    fs->device->read(fs->device, boot_sector, fs->partition_begin_offset, 1);

    fat_boot_sector_t* bs = (fat_boot_sector_t*)boot_sector;

    //Starting from 2..
    index -= 2;
    uint32_t cluster_start_sector = bs->table_count + (index * bs->sectors_per_cluster) + 1;

    fs->device->read(fs->device, buf, cluster_start_sector, bs->sectors_per_cluster);
}

inline void write_cluster(file_system_t* fs, fat_boot_sector* boot_sector, uint32_t index, void* buf)
{
    fs->partition_begin_offset
}

static status_t get_root_dir(file_system_t* fs, void* root_entry)
{
    if(fs == NULL || root_entry == NULL)
    {
        return INVALID_ARGUMENT;
    }

    uint32_t partition_offset = fs->partition_begin_offset;
    uint8_t buffer[fs->device->sector_size];

    //Read the boot sector
    fs->device->read(fs->device, buffer, partition_offset, 1);

    fat_boot_sector_t* bs = (fat_boot_sector_t*)buffer;
    fat_extBS_32_t* ext_bs = (fat_extBS_32_t*)bs->extended_section;

    uint32_t cluster_size = bs->sectors_per_cluster * fs->device->sector_size;
    uint32_t root_offset = 0;
    root_offset = partition_offset + bs->table_count;
    root_offset += (ext_bs->root_cluster - 2) * cluster_size; // We start numbering from 2

    //Assume root dir takes only 1 cluster
    fs->device->read(fs->device, root_entry, root_offset, bs->sectors_per_cluster);
}

/*
 *  Search for an entry in a directory whose named 'name'.
 */
static status_t search_for_entry_in_dir(fat_file_entry_t** file_entry_to_update,
                                        fat_file_entry_t* entries,
                                        uint32_t entries_size,
                                        char* name)
{
    for(fat_file_entry_t* entry = entries;
        entry < ((unsigned int)entries + entries_size);
        entry++)
    {
        if(strcmp(entry->file_name, name) == 0)
        {
            *file_entry_to_update = entry;
            return SUCCESS;
        }
    }

    return NO_SUCH_FILE;
}

/*
 * ROFL
 */
static void* fat_walk(file_system_t* fs, uint32_t fat_start_sector,
                      uint32_t fat_num_sectors, uint32_t starting_cluster, uint32_t cluster_size)
{
    if(starting_cluster < 3)
    {
        return NULL;
    }

    uint8_t fat_table[fat_num_sectors * fs->device->sector_size];

    //Read the fat table to ram.
    fs->device->read(fs->device, fat_table, fat_start_sector, fat_num_sectors);
    uint32_t* fat_table_entry = (uint32_t*)fat_table;

    //index 0 == cluster 3 (root is 2)
    fat_table_entry += starting_cluster;
    uint32_t size = 0;
    //Calculate the size of the buffer needed
    while(*fat_table_entry != FAT_EOC)
    {
        size += cluster_size;
        fat_table_entry = &fat_file_entry[*fat_table_entry];
    }

    if(size == 0)
    {
        return NULL;
    }

    char* buffer = kmalloc(size);
    uint8_t cluster[cluster_size];

    fat_table_entry = (uint32_t*)fat_table;
    fat_table_entry += starting_cluster;
    uint32_t cluster_index = starting_cluster;

    do
    {
        read_cluster(fs, cluster_index, cluster);
        memcpy(buffer, cluster, cluster_size);
        buffer += cluster_size;
        cluster_index = *fat_table_entry;
    }
    while(*fat_table_entry != FAT_EOC);
}

static status_t get_inode(file_system_t* fs, fs_node_t* fs_node, char* path)
{
    if(fs == NULL || fs_node == NULL || path == NULL)
    {
        return INVALID_ARGUMENT;
    }

    //If we're asking for the root dir.
    if(strlen(path) == 1)
    {
        memcpy(fs_node->name, "/", 2);
        fs_node->fs = fs;
        fs_node->uid = 0;
        fs-node->gid = 0;
        fs_node->flags = requested_entry->file_attributes;
        fs_node->starting_cluster = 0;
        fs_node->impl_defined = requested_entry->starting_cluster;
        fs_node->length = requested_entry->file_size;
        return SUCCESS;
    }

    uint8_t entries[fs->device->sector_size];

    //First get the root directory.
    if(get_root_dir(fs, entries) == ERROR)
    {
        return ERROR;
    }

    //Count the depth.
    uint8_t depth = 0;
    for(int i = 1; i < strlen(path); i++)
    {
        if(*path == '/')
        {
            depth++;
        }
    }

    fat_file_entry_t* file_entries = NULL;
    fat_file_entry_t* requested_entry = NULL;

    file_entries = (fat_file_entry_t*)entries;
    bool_t is_root_dir = true;

    char* token = strtok(path, "/");

    do
    {
        if (search_for_entry_in_dir(&requested_entry, file_entries, fs->device->sector_size, token)
            == NO_SUCH_FILE)
        {
            return INVALID_PATH;
        }

        if(depth == 1)
        {
            //Found..
            memcpy(fs_node->name, requested_entry->file_name, strlen(requested_entry->file_name) + 1);
            fs_node->fs = fs;
            fs_node->uid = 0;
            fs-node->gid = 0;
            fs_node->flags = requested_entry->file_attributes;
            //Keep here the starting cluster.
            fs_node->impl_defined = requested_entry->starting_cluster;
            fs_node->length = requested_entry->file_size;
            return SUCCESS;
        }

        if (!IS_DIRECTORY(requested_entry->file_attributes))
        {
            return INVALID_PATH;
        }

        --depth;

        if (!is_root_dir)
        {
            kfree(file_entries);
        }
        else
        {
            is_root_dir = false;
        }

        //You're facing a directory. let's gather all of its content into the RAM
        //And walk on it.
        file_entries = fat_walk(fs, fat_start_sector, fat_num_sectors, requested_entry->starting_cluster);
    }
    while((token = strtok(NULL, "/")) != NULL);
}

fs_node_t* open(file_system_t* fs, char* path)
{
    get_inode(fs, path);
}

void close(file_system_t* fs, fs_node_t* node)
{
}

void read(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer)
{
}

void write(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer)
{
}

status_t create_file(file_system_t* fs, char* path)
{
    status_t return_status;
    inode_t parent_inode = {0};

    //Get the inode of the father directory.
    if ((return_status = get_inode(fs, &parent_inode, path)) != SUCCESS)
    {
        return return_status;
    }

}

void create_directory(char* path)
{
    //Get the inode of the father directory.
}
#endif
