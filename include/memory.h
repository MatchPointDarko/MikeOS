#ifndef MIKE_OS_MEMORY_H
#define MIKE_OS_MEMORY_H

#include "common.h"

void memset(char value, char* addr, unsigned long length);
char extract_bit(char value, int bit_index);
void turn_bit_on(char* value, int bit_index);
void turn_bit_off(char* value, int bit_index);
unsigned int msb_index(unsigned char value);
void memcpy(void* dst, void* src, uint32_t num);

#endif //MIKE_OS_MEMORY_H
