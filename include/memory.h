#ifndef MIKE_OS_MEMORY_H
#define MIKE_OS_MEMORY_H

void memset(char value, char* addr, unsigned long length);
char extract_bit(char value, int bit_index);
void turn_bit_on(char* value, int bit_index);
void turn_bit_off(char* value, int bit_index);
unsigned int get_msb_index(unsigned char value);
void memcpy(void* dst, void* src, unsigned int num);
void turn_bits_off(char* value_ptr, int bit_index, int num_bits);
void turn_bits_on(char* value_ptr, int bit_index, int num_bits);

#endif //MIKE_OS_MEMORY_H
