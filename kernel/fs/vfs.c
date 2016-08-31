#include "common.h"
#include "panic.h"
#include "kmalloc.h"
#include "memory.h"
#include "string.h"
#include "fs.h"

#define IS_DIRECTORY(flags) (flags & 0x7)
#define MAX_FILE_NAME_LENGTH (128)
#define PATH_SEPERATOR ('/')

typedef enum vfs_error
{
    NO_SUCH_DIRECTORY,
    NO_SUCH_FILE,
    INVALID_ARGUMENT

} vfs_error_t;

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

typedef enum file_modes
{ READ = 1, WRITE = 2, EXECUTE = 4} file_modes_t;

typedef struct fs_node
{
    char name[MAX_FILE_NAME_LENGTH];     // The filename.
    uint32_t permissions;        // The permissions mask.

    uint32_t uid;         // The owning user.
    uint32_t gid;         // The owning group.
    uint32_t flags;       // Includes the node type. See #defines above.
    uint32_t inode;       // This is device-specific - provides a way for a filesystem to identify files.
    uint32_t length;      // Size of the file, in bytes.
    uint32_t impl_defined;        // An implementation-defined number.

    void(*read) (struct fs_node* node, uint32_t offset, uint32_t length, void* buffer);
    void(*write) (struct fs_node* node, uint32_t offset, uint32_t length, void* buffer);
    void(*open) (struct fs_node* node, file_modes_t permissions);
    void(*close) (struct fs_node* node);
    struct fs_node* (*readdir) (struct fs_node*);
    struct fs_node* (*finddir) (struct fs_node*, const char* path);

    file_system_t* fs;  //fs containing the node.
    struct fs_node *ptr; // Used by mountpoints and symlinks.
} fs_node_t;


fs_node_t* root_node = NULL;

/*
 * Initialize virtual filesystem.
 * Allocate a root node.
 */
void vfs_init()
{
    if(!(root_node = kmalloc(sizeof(fs_node_t))))
    {
        //This should never happen.
        kernel_panic();
    }

    memcpy(root_node->name, "/", sizeof("/"));

    root_node->gid = root_node->gid = 0;
}

#if 0
/*
 * Mount a file system in the given path.
 */
vfs_error_t vfs_mount(const char* path, file_system_t* fs)
{
    if(path == NULL || fs == NULL || !is_valid_path(path))
    {
        //Stop bullshiting me!
        return INVALID_ARGUMENT;
    }

    //TODO: Currently assumes path isn't relative
    //TODO: Get back here and change it when cwd is implemented.

    if(strcmp(path, "/") == 0)
    {
        //Trying to mount the root node, he has balls.
        if(fs->write && fs->read && fs->open && fs->close && fs->finddir)
        {
            root_fs->write = fs->write;
            root_fs->read = fs->read;
            root_fs->finddir = fs->finddir;
            root_fs->open = fs->open;
            root_fs->close = fs->close;
        }
        else
        {
            //Stop bullshiting me!
            return false;
        }
    }

    uint32_t depth = count_chars(path, PATH_SEPERATOR);
    const char* mount_name = NULL;
    fs_node_t* node = root_node;

    const char* path_token = strtok(path, PATH_SEPERATOR);
    do
    {
        node = node->finddir(node, path_token);
    }
    while(depth - 1 != 0 && (path_token = strtok(NULL, PATH_SEPERATOR) != NULL));

    //Now we got the node of the directory.
    mount_name = path_token;
}
#endif
