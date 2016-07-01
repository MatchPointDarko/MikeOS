#include "stdio.h"
#include "memory.h"
#include "physical_mm_manager.h"
#include "external_linker_symbols.h"

#define PAGE_SIZE 4096 //4KB
#define NO_AVAILABLE_PAGE -1
#define PAGE_UNAVAILABLE 0x00
#define PAGE_AVAILABLE 0xFF
#define PAGE_ALIGN(value) ((unsigned long)value) & ~(PAGE_SIZE - 1)
#define MANAGED_MEMORY_PAGE_ALIGN(value) (PAGE_ALIGN(value)) + \
                                         (managed_memory_start_addr - \
                                         (PAGE_ALIGN(managed_memory_start_addr)))

#define NULL (void*)0

static char* bitmap = (char*)0;
unsigned long current_free_page_index = 0;
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
                if(map_iterator->addr <= (unsigned long)&kernel_start &&
                     (unsigned long)&kernel_start <= (map_iterator->addr + map_iterator->len))
                {
                    //bitmap will contain virtual address.
                    bitmap = ((char*)&kernel_end + KERNEL_VIRTUAL_OFFSET);
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

    memset(PAGE_AVAILABLE, bitmap, bitmap_size);
}


void memory_manager_init(multiboot_memory_map_t* map_addr, unsigned int map_length)
{
    map_memory(map_addr, map_length);
}

/* Get the byte index form of the bit index.
 * i.e.:
 * bit_index == 7 -> byte_index == 0
 * bit_index == 15 -> byte_index == 1
 * bit_index == 21 -> byte_index == 2
 */
static inline unsigned long bit_index_to_byte_index(unsigned long bit_index)
{
    return bit_index / (sizeof(char) * 8);
}

/* Set the current free page index to the next free page,
 * Since it was allocated.
 */
static void set_next_free_page_index()
{
    unsigned long bitmap_index = bit_index_to_byte_index(current_free_page_index);
    for(int i = bitmap_index; i < bitmap_size; i++)
    {
        if(bitmap[i] != 0x00)
        {
            current_free_page_index = (i * 8) + msb_index(bitmap[i]);
            return;
        }
    }

    current_free_page_index = NO_AVAILABLE_PAGE;
}

void* allocate_physical_page()
{
    if(current_free_page_index == NO_AVAILABLE_PAGE)
    {
        return NULL;
    }

    unsigned char bit_index = current_free_page_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(current_free_page_index);

    turn_bit_off(&bitmap[bitmap_index], bit_index);

    void* free_page_addr = (void*)(managed_memory_start_addr + (PAGE_SIZE * (current_free_page_index)));

    set_next_free_page_index();

    return free_page_addr;
}

static inline unsigned long address_to_bit_index(void* addr)
{
    unsigned long aligned_addr = MANAGED_MEMORY_PAGE_ALIGN(addr);
    return (aligned_addr - managed_memory_start_addr) / PAGE_SIZE;
}

void free_physical_page(void* page_addr)
{
    if((unsigned long)page_addr < managed_memory_start_addr)
    {
        return;
    }

    unsigned long bit_index = address_to_bit_index(page_addr);

    unsigned long bit_location = bit_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(bit_index);

    turn_bit_on(&bitmap[bitmap_index], bit_location);

    //Check bit index is lower than the current free page
    if(bit_index < current_free_page_index)
    {
        current_free_page_index = bit_index;
    }
}
