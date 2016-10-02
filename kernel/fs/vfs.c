/*
 * MikeOS: Virtual File System Implementation.
 */

#include <common.h>
#include <panic.h>
#include <mm/kmalloc.h>
#include <libc/memory.h>
#include <libc/string.h>
#include <fs/vfs.h>
#include <data_structures/list.h>
#include <logging/logger.h>

typedef struct mount_point
{
    char mount_path[MAX_PATH_LENGTH];
    file_system_t* mounted_fs;
    list_t children;

} mount_point_t;

static mount_point_t* mount_points = NULL;
static mount_point_t* root_mount_point = NULL;
block_device_t* root_device = NULL;

/*
 * Initialize virtual filesystem.
 * Allocate a root node.
 */
void vfs_init()
{
    if(!(mount_points = kmalloc(sizeof(mount_point_t))))
    {
        //This should never happen.
        kernel_panic();
    }

    memcpy(mount_points->mount_path, "/", sizeof("/"));

    mount_points->mounted_fs = NULL;

    if(!initialize_list(&mount_points->children))
    {
        kernel_panic();
    }

    root_mount_point = mount_points;

    log_print(LOG_DEBUG, "Successfully initialized the virtual file system");
}

/*
 * Mount a file system in the given path.
 */
error_code_t vfs_mount(const char* path, file_system_t* fs)
{
    if(path == NULL || fs == NULL)
    {
        //Stop bullshiting me!
        return INVALID_ARGUMENT;
    }

    //TODO: Currently assumes path isn't relative
    //TODO: Get back here and change it when cwd is implemented.

    if(strcmp(path, "/") == 0)
    {
        //Trying to mount the root node, he has balls.
        if(fs->read && fs->open && fs->close)
        {
            root_mount_point->mounted_fs = fs;
            return SUCCESS;
        }
        else
        {
            //Stop bullshiting me!
            return FAILURE;
        }
    }
    else
    {
        //If we don't have a root fs yet, this is pointless.
        if(root_mount_point->mounted_fs == NULL)
        {
            return NO_SUCH_DIRECTORY;
        }
    }

    /*
    uint32_t depth = count_chars(path, PATH_SEPERATOR);
    const char* mount_name = NULL;
    fs_node_t* node = root_node;

    const char* path_token = strtok(path, PATH_SEPERATOR);
    do
    {
        node = node->fs->finddir(node, path_token);
    }
    while(depth - 1 != 0 && (path_token = strtok(NULL, PATH_SEPERATOR) != NULL));

    //Now we got the node of the directory.
    mount_name = path_token;
    */
}

static uint32_t path_depth(const char* path)
{
    if(path == NULL)
    {
        return -1;
    }

    //Assumes the path is valid!

    uint32_t counter = 0;

    while(*path != '\0')
    {
        if(*path == '/')
        {
            counter++;
        }

        path++;
    }

    return counter;
}

mount_point_t* alloc_mount_point(char* path, file_system_t* fs)
{
    mount_point_t* mount_point = kmalloc(sizeof(mount_point_t));

    if(mount_point == NULL)
    {
        return NULL;
    }

    memcpy(mount_point->mount_path, path, strlen(path) + 1);
    mount_point->mounted_fs = fs;

    return mount_point;
}

fs_node_t* vfs_open(const char* path)
{
    //Find the filesystem.
    char *mount_point = NULL;
    char *token = NULL;

    //Walk on the mounts tree.
    list_t tree_nodes;
    mount_point_t* current = NULL;

    if(!initialize_list(&tree_nodes))
    {
        goto clean_up;
    }

    if(!add_element(&tree_nodes, mount_points))
    {
        goto clean_up;
    }

    do
    {
        const char* specific_path = NULL;
        if((current = pop_element(&tree_nodes, 0)) != NULL)
        {
            mount_point_t* child = NULL;
            //Add its children
            while(child = iter_list(&mount_points->children))
            {
                //Do we have a winner?
                if((specific_path = strstr(path, child->mount_path)) != NULL)
                {
                    specific_path = specific_path + strlen(child->mount_path);
                    if(!add_element(&tree_nodes, mount_points))
                    {
                        goto clean_up;
                    }

                }
            }

        }

    }
    while (!is_empty(&tree_nodes));

clean_up:
    free_list(&tree_nodes);
    if(current != NULL)
    {
        return current->mounted_fs->open(current->mounted_fs, path);
    }
    else
    {
        return NULL;
    }
}

void vfs_unmount(file_system_t* fs)
{}

uint32_t vfs_read(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer)
{
    if(node == NULL)
    {
        return 0;
    }

    return node->fs->read(node, offset, length, buffer);
}

uint32_t vfs_write(fs_node_t* node, uint32_t offset, uint32_t length, void* buffer)
{}

void vfs_close(fs_node_t* node)
{
    node->fs->close(node);
}
