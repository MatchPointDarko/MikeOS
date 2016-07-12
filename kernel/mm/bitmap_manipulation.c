/* Get the byte index form of the bit index.
 * i.e.:
 * bit_index == 7 -> byte_index == 0
 * bit_index == 15 -> byte_index == 1
 * bit_index == 21 -> byte_index == 2
 */
#include "memory.h"
#include "bitmap_manipulation.h"

unsigned long bit_index_to_byte_index(unsigned long bit_index)
{
    return bit_index / (sizeof(char) * 8);
}

unsigned long address_to_bit_index(void* addr, void* start_address)
{
    unsigned long aligned_addr = MANAGED_MEMORY_PAGE_ALIGN(addr, (unsigned long) start_address);
    return (aligned_addr - (unsigned long)start_address) / PAGE_SIZE;
}

void set_next_free_page_index(char* bitmap, unsigned long bitmap_size, unsigned long* current_free_page_index)
{
    unsigned long bitmap_index = bit_index_to_byte_index(*current_free_page_index);
    for(int i = bitmap_index; i < bitmap_size; i++)
    {
        if(bitmap[i] != 0x00)
        {
            *current_free_page_index = (i * 8) + msb_index(bitmap[i]);
            return;
        }
    }

    *current_free_page_index = NO_AVAILABLE_PAGE;
}
