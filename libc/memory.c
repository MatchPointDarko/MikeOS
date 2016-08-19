/*
 * MikeOS: Memory helper fuctions.
 */

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
   return (value >> (bit_index + (sizeof(char) * 8) - 1)) & 0x1;
}

unsigned int msb_index(unsigned char value)
{
    unsigned int index = 0;

    while(value >>= 1)
    {
        index++;
    }

    return (sizeof(char) * 8 - 1) - index;
}

void turn_bit_on(char* value_ptr, int bit_index)
{
    *value_ptr |= (0x80 >> bit_index);
}

void turn_bit_off(char* value_ptr, int bit_index)
{
    *value_ptr &= ~(0x80 >> bit_index);
}
