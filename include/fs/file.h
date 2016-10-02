#ifndef MIKE_OS_FILE_H
#define MIKE_OS_FILE_H

#include <fs/vfs.h>
#include <common.h>

file_t* file_open(const char* path, const char* mode);
error_code_t file_close(file_t* file);
uint32_t file_write(file_t* file, uint8_t* buffer, uint32_t length);
uint32_t file_read(file_t* file, uint8_t* buffer, uint32_t length);

#endif //MIKE_OS_FILE_H
