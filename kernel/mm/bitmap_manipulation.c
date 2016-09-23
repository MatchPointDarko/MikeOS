/*
 * MikeOS: Helper functions for bit manipulations,
 *         mainly helps the memory managers bitmaps.
 */

#include "memory.h"
#include "bitmap_manipulation.h"

unsigned long bit_index_to_byte_index(unsigned long bit_index)
{
    return bit_index / (sizeof(char) * 8);
}

unsigned long address_to_bit_index(uint32_t addr, uint32_t start_address)
{
    unsigned long aligned_addr = MANAGED_MEMORY_PAGE_ALIGN(addr, start_address);
    return (aligned_addr - start_address) / PAGE_SIZE;
}

/*
 * Iterate on the bitmap, set the lowest one bit you find to the current free page.
 */
void set_next_free_page_index(char* bitmap, unsigned long bitmap_size, unsigned long* current_free_page_index)
{
    for(int i = 0; i < bitmap_size; i++)
    {
        if(bitmap[i] != 0x00)
        {
            *current_free_page_index = (i * 8) + get_msb_index(bitmap[i]);
            return;
        }
    }

    *current_free_page_index = NO_AVAILABLE_PAGE;
}
