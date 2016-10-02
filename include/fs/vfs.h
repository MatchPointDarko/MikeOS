#ifndef MIKE_OS_VFS_H
#define MIKE_OS_VFS_H

/**** File systems supported ****/
#include <fs/initrd.h>
/*******************************/

#include <error_codes.h>
#include <common.h>
#include <dev/device.h>

#define IS_DIRECTORY(flags) (flags & 0x7)
#define MAX_FILE_NAME_LENGTH (128)
#define MAX_PATH_LENGTH (300)
#define PATH_SEPERATOR ('/')

typedef struct file_system file_system_t;

typedef struct fs_node
{
    char name[MAX_FILE_NAME_LENGTH];
    uint32_t permissions;

    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t size;
    uint32_t fs_specific;

    file_system_t* fs;  //fs containing the node.
    struct fs_node *ptr; // Used by mountpoints and symlinks.
} fs_node_t;

typedef struct file_system
{
    char name[20];
    block_device_t* device;
    uint32_t fs_specific;
    uint32_t partition_begin_offset;
    uint32_t partition_end_offset;
    uint32_t (*read) (struct fs_node* node, uint32_t offset, uint32_t length, void* buffer);
    uint32_t (*write) (struct fs_node* node, uint32_t offset, uint32_t length, void* buffer);
    fs_node_t* (*open) (struct file_system* fs, const char* path);
    void (*close) (struct fs_node* node);
    struct fs_node* (*readdir) (struct fs_node*, const char* path);

} file_system_t;

typedef struct file
{
    //TODO: uint32_t flags;
    uint32_t mode;
    uint32_t offset;
    fs_node_t* inode;
} file_t;

typedef enum file_mode
{
    READ = 1,
    WRITE = 2,
    APPEND = 4,
    CREATE_FILE = 8,
    TRUNCATE_TO_ZERO = 16
} file_mode_t;


enum fs_node_types
{
    FS_FILE = 0x01,
    FS_DIRECTORY,
    FS_CHARDEVICE,
    FS_BLOCKDEVICE,
    FS_PIPE,
    FS_SYMLINK,
    FS_MOUNTPOINT = 0x08
};

extern block_device_t* root_device;

/**** Function prototypes ****/
void vfs_init();
error_code_t vfs_mount(const char* path, file_system_t* fs);
uint32_t vfs_read(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer);
uint32_t vfs_write(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer);
fs_node_t* vfs_open(const char* path);
void vfs_close(fs_node_t* node);

#endif //MIKE_OS_VFS_H
