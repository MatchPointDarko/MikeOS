#include "logger.h"
#include "stdio.h"
#include "memory.h"
#include "logger.h"
#include "bitmap_manipulation.h"
#include "physical_mm_manager.h"
#include "external_linker_symbols.h"
#include "common.h"

#define PAGE_SIZE 4096 //4KB
#define PAGE_UNAVAILABLE 0x00
#define PAGE_AVAILABLE 0xFF

/*TODO: The memory mapping assumes there are no HOLES in ram.
 * i.e., computes the next available physical page, according
 * to an index and the base, consider holes in ram.
 */
static char* bitmap = NULL;
static unsigned long current_free_page_index = 0;
static unsigned long bitmap_size = 0;
static unsigned long managed_memory_start_addr = 0;
static unsigned long free_memory_size = 0;

/* Map the RAM, using the GRUB memory map. */
void map_memory(multiboot_memory_map_t* map_addr, unsigned int map_length)
{
    int found_first_available = 0;

    for(multiboot_memory_map_t* map_iterator = map_addr;
        map_iterator < (map_addr + map_length);
        map_iterator = (multiboot_memory_map_t*)((unsigned int)map_iterator + map_iterator->size + 4))
    {
        if(map_iterator->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            if(!found_first_available)
            {
                if(map_iterator->addr <= KERNEL_START &&
                     KERNEL_START <= (map_iterator->addr + map_iterator->len))
                {
                    //bitmap will contain virtual address.
                    bitmap = (char*)(KERNEL_END + KERNEL_VIRTUAL_OFFSET);
                    free_memory_size += map_iterator->len - KERNEL_SIZE;
                    found_first_available = 1;
                }
            }
            else
            {
                free_memory_size += map_iterator->len;
            }
        }
    }
    bitmap_size = (free_memory_size / PAGE_SIZE) / 8;

    //Reserve for bitmap.
    free_memory_size -= bitmap_size;

    //Managed must be physical!!!
    managed_memory_start_addr = ((unsigned long)bitmap + bitmap_size) - KERNEL_VIRTUAL_OFFSET;

    //Align managed memory address to page boundry
    managed_memory_start_addr = NEXT_PAGE_BOUNDARY(managed_memory_start_addr);

    memset(PAGE_AVAILABLE, bitmap, bitmap_size);
    log_print(LOG_DEBUG, "Physical Memory mapped successfuly!");
    log_print(LOG_DEBUG, "bitmap virtual address: %x", bitmap);
    log_print(LOG_DEBUG, "managed physical memory start address: %x", managed_memory_start_addr);
}


void memory_manager_init(multiboot_memory_map_t* map_addr, unsigned int map_length)
{
    map_memory(map_addr, map_length);
}

/* Allocate a single page. */
void* allocate_physical_page()
{
    if(current_free_page_index == NO_AVAILABLE_PAGE)
    {

        return NULL;
    }

    // Bit index in a byte
    unsigned char bit_index = current_free_page_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(current_free_page_index);

    turn_bit_off(&bitmap[bitmap_index], bit_index);

    void* free_page_addr = (void*)(managed_memory_start_addr + (PAGE_SIZE * (current_free_page_index)));

    set_next_free_page_index(bitmap, bitmap_size, &current_free_page_index);

    return free_page_addr;
}

void free_physical_pages(void* page_addr)
{
    if((unsigned long)page_addr < managed_memory_start_addr)
    {
        //TODO:panic..
        return;
    }

    unsigned long bit_index = address_to_bit_index(page_addr, (void *) managed_memory_start_addr);

    unsigned long bit_location = bit_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(bit_index);

    turn_bit_on(&bitmap[bitmap_index], bit_location);

    //Check bit index is lower than the current free page
    if (bit_index < current_free_page_index)
    {
        current_free_page_index = bit_index;
    }
}
