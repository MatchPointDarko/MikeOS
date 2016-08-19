#include "common.h"

#if 0
enum
{
    FS_FILE = 0x01,
    FS_DIRECTORY,
    FS_CHARDEVICE,
    FS_BLOCKDEVICE,
    FS_PIPE,
    FS_SYMLINK,
    FS_MOUNTPOINT = 0x08
};

typedef struct fs_node
{
    char name[128];     // The filename.
    uint_32 mask;        // The permissions mask.
    uint_32 uid;         // The owning user.
    uint_32 gid;         // The owning group.
    uint_32 flags;       // Includes the node type. See #defines above.
    uint_32 inode;       // This is device-specific - provides a way for a filesystem to identify files.
    uint_32 length;      // Size of the file, in bytes.
    uint_32 impl;        // An implementation-defined number.
    //read_type_t read;
    //write_type_t write;
    //open_type_t open;
    //close_type_t close;
    //readdir_type_t readdir;
    //finddir_type_t finddir;
    struct fs_node *ptr; // Used by mountpoints and symlinks.
} fs_node_t;

#define IS_DIRECTORY(flags) (flags & 0x7)

fs_node_t* root = 0;

uint_32 read_fs(fs_node_t* node, uint_32 offset, uint_32 size, uint_8* buffer)
{
    if(node->read != NULL)
    {
        return node->read();
    }
}
#endif
