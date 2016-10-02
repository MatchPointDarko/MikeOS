/*
 * MikeOS: Memory helper functions.
 */

#include <libc/memory.h>

void memcpy(void* dst, void* src, unsigned int num)
{
    char *dst_it = dst, *src_it = src;

    for(int i = 0; i < num; i++)
    {
        dst_it[i] = src_it[i];
    }
}

void memset(char value, char* addr, unsigned long length)
{
    for(int i = 0; i < length; i++)
        addr[i] = value;
}

/* Extracts a bit from a byte, the counting starts:
 * [0][1][2][3][4][5][6][7]
 *
 * Where 0 is MSB, 7 is LSB.
 */
char extract_bit(char value, int bit_index)
{
    return (value >> ((sizeof(char) * 8) - bit_index - 1)) & 0x1;
}

/*
 * MSB Index.
 * << Low indices
 * >> High indices
 */
unsigned int get_msb_index(unsigned char value)
{
    unsigned int index = 0;

    while(value)
    {
        index++;
        value >>= 1;
    }

    return (sizeof(char) * 8) - index;
}

void turn_bit_on(char* value_ptr, int bit_index)
{
    *value_ptr |= (0x80 >> bit_index);
}

void turn_bit_off(char* value_ptr, int bit_index)
{
    *value_ptr &= ~(0x80 >> bit_index);
}

/*
 * << Lower(MSB)
 * >> Higher(LSB)
 */
void turn_bits_off(char* buffer, int bit_index, int num_bits)
{
    unsigned int byte_index = bit_index / 8;
    unsigned int num_bytes = num_bits / 8 + 1;

    for(int i = bit_index; i < bit_index + num_bits; i++)
    {
        turn_bit_off(&buffer[i / 8], i % 8);
    }
}

/*
 * << Lower(MSB)
 * >> Higher(LSB)
 */
void turn_bits_on(char* buffer, int bit_index, int num_bits)
{
    unsigned int byte_index = bit_index / 8;
    unsigned int num_bytes = num_bits / 8 + 1;

    for(int i = bit_index; i < bit_index + num_bits; i++)
    {
        turn_bit_on(&buffer[i / 8], i % 8);
    }
}
