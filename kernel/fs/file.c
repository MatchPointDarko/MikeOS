#include <fs/file.h>
#include <error_codes.h>
#include <libc/string.h>
#include <mm/kmalloc.h>
#include <libc/memory.h>

static char* possible_modes[] = {"r", "rb", "w", "wb", "a", "ab", "r+", "rb+", "r+b",
                                 "w+", "wb+", "w+b", "a+", "ab+", "a+b"};

static file_mode_t modes_bits[] = {(READ), (READ),
                              (WRITE | CREATE_FILE | TRUNCATE_TO_ZERO),
                              (WRITE | CREATE_FILE | TRUNCATE_TO_ZERO),
                              (APPEND | CREATE_FILE), (APPEND | CREATE_FILE),
                              (READ | WRITE), (READ | WRITE), (READ | WRITE),
                              (TRUNCATE_TO_ZERO | CREATE_FILE | READ | WRITE),
                              (TRUNCATE_TO_ZERO | CREATE_FILE | READ | WRITE),
                              (TRUNCATE_TO_ZERO | CREATE_FILE | READ | WRITE),
                              (APPEND | CREATE_FILE),
                              (APPEND | CREATE_FILE),
                              (APPEND | CREATE_FILE)};

file_t* file_open(const char* path, const char* mode)
{
    if(path == NULL)
    {
        return NULL;
    }

    //In case mode is read only section:
    uint32_t mode_size = strlen(mode);
    char mode_buf[mode_size + 1];
    memcpy(mode_buf, (char *)mode, mode_size);

    mode_buf[mode_size] = '\0';

    file_t* file = NULL;

    for(int i = 0; i < SIZE_OF_ARRAY(possible_modes); i++)
    {
        if(strncmp(mode_buf, possible_modes[i], 3) == 0)
        {
            fs_node_t* inode = vfs_open(path);
            if(inode != NULL)
            {
                file = kmalloc(sizeof(file_t));
                file->inode = inode;
                file->mode = modes_bits[i];
                file->offset = 0;
                //TODO: file->flags
            }
            break;
        }
    }

    return file;
}

/*
 * Read 'length' bytes from the file.
 * Returns the number of bytes read.
 */
uint32_t file_read(file_t* file, uint8_t* buffer, uint32_t length)
{
    //TODO: Check if have read permissions..
    if(buffer == NULL || file == NULL)
    {
        return 0;
    }

    if(length > (file->inode->size - file->offset))
    {
        length = (file->inode->size - file->offset);
    }

    uint32_t bytes_read = vfs_read(file->inode, file->offset, length, buffer);

    file->offset += bytes_read;

    return bytes_read;
}

/*
 * Writes 'length' bytes to the file.
 * Returns the number of bytes written.
 */
uint32_t file_write(file_t* file, uint8_t* buffer, uint32_t length)
{
    //TODO: Check if have write permissions..
    if(buffer == NULL || file == NULL)
    {
        return INVALID_ARGUMENT;
    }

    int32_t bytes_written = file->inode->fs->write(file->inode, file->offset, length, buffer);

    file->offset += bytes_written;

    return bytes_written;
}


error_code_t file_close(file_t* file)
{
    if(file == NULL)
    {
        return INVALID_ARGUMENT;
    }

    vfs_close(file->inode);

    kfree(file);

    return SUCCESS;
}
