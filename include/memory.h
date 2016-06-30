//
// Created by sourcer on 25/06/16.
//

#ifndef MIKE_OS_MEMORY_H
#define MIKE_OS_MEMORY_H
void memset(char value, char* addr, unsigned long length);
char extract_bit(char value, int bit_index);
void turn_bit_on(char* value, int bit_index);
void turn_bit_off(char* value, int bit_index);
unsigned int msb_index(char value);

#endif //MIKE_OS_MEMORY_H
