#ifndef MIKE_OS_FILE_H
#define MIKE_OS_FILE_H

#include <fs/vfs.h>
#include <common.h>

typedef enum seek_type
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END

} seek_type_t;

file_t* file_open(const char* path, const char* mode);
error_code_t file_close(file_t* file);
uint32_t file_write(file_t* file, uint8_t* buffer, uint32_t length);
uint32_t file_read(file_t* file, uint8_t* buffer, uint32_t length);
error_code_t file_seek(file_t* file, int32_t jmp, seek_type_t seek_type);

#endif //MIKE_OS_FILE_H
