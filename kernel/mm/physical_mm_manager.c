#include "logger.h"
#include "panic.h"
#include "stdio.h"
#include "memory.h"
#include "logger.h"
#include "bitmap_manipulation.h"
#include "physical_mm_manager.h"
#include "common_constants.h"
#include "common.h"

typedef struct physical_mem_region
{
    char* bitmap;
    unsigned int bitmap_size;
    unsigned long size;
    unsigned long base_address;
    unsigned int next_free_page;

} physical_mem_region_t;

static unsigned int number_of_regions = 0;
static physical_mem_region_t* physical_memory_regions = NULL;

static char* bitmap = NULL;
static unsigned long current_free_page_index = 0;
static unsigned long bitmap_size = 0;
static unsigned long managed_memory_start_addr = 0;
static unsigned long free_memory_size = 0;

/* Map the RAM, using the GRUB memory map. */
void map_memory(multiboot_memory_map_t* map_addr, unsigned int map_length)
{
    unsigned long bitmap_size = 0;
    bool_t found_first_available_region = false;

    log_print(LOG_DEBUG, "Starting to map the RAM");
    for(multiboot_memory_map_t* map_iterator = map_addr;
        map_iterator < (map_addr + map_length);
        map_iterator = (multiboot_memory_map_t*)((unsigned int)map_iterator + map_iterator->size + 4))
    {
        if(map_iterator->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            if(found_first_available_region == false)
            {
                // Start managing memory only after the kernel
                if(map_iterator->addr <= KERNEL_START &&
                     KERNEL_START <= (map_iterator->addr + map_iterator->len))
                {
                    log_print(LOG_DEBUG, "Address: %x --- Length: %d", map_iterator->addr,
                                                                       map_iterator->len);
                    physical_memory_regions = (physical_mem_region_t*)(KERNEL_END + KERNEL_VIRTUAL_OFFSET);
                    physical_memory_regions[number_of_regions].size = map_iterator->len - KERNEL_SIZE;
                    bitmap_size += map_iterator->len - KERNEL_SIZE;

                    number_of_regions++;

                    found_first_available_region = true;
                }
            }
            else
            {
                log_print(LOG_DEBUG, "Address: %x --- Length: %d", map_iterator->addr,
                                                                       map_iterator->len);
                physical_memory_regions[number_of_regions].size = map_iterator->len;
                physical_memory_regions[number_of_regions].base_address = map_iterator->addr;
                bitmap_size += map_iterator->len;

                number_of_regions++;
            }
        }
    }

    //Allocate a bitmap
    char* bitmap = (char *)&physical_memory_regions[number_of_regions];

    bitmap_size /= PAGE_SIZE;
    bitmap_size /= sizeof(char) * 8;

    memset(PAGE_AVAILABLE, (void *)bitmap, bitmap_size);

    char* physical_bitmap_address = (char *)((unsigned long) bitmap - KERNEL_VIRTUAL_OFFSET);
    physical_memory_regions[0].base_address = NEXT_PAGE_BOUNDARY(physical_bitmap_address + bitmap_size);
    physical_memory_regions[number_of_regions].size -= number_of_regions * sizeof(physical_mem_region_t);
    physical_memory_regions[number_of_regions].size -= bitmap_size;

    unsigned int last_size = 0;
    unsigned int region_offset = 0;

    for(int i = 0; i < number_of_regions; i++)
    {
        physical_memory_regions[i].bitmap = bitmap + region_offset;
        physical_memory_regions[i].next_free_page = region_offset;
        physical_memory_regions[i].bitmap_size = physical_memory_regions[i].size / PAGE_SIZE / (sizeof(char) * 8);
        region_offset += physical_memory_regions[i].bitmap_size;
    }

    log_print(LOG_DEBUG, "Physical Memory mapped successfuly!");
}


void memory_manager_init(multiboot_memory_map_t* map_addr, unsigned int map_length)
{
    map_memory(map_addr, map_length);
}

/* Allocate a single page. */
void* allocate_physical_page()
{
    for(int i = 0; i < number_of_regions; i++)
    {
        physical_mem_region_t* current_region = &physical_memory_regions[i];
        if(current_region->next_free_page == NO_AVAILABLE_PAGE)
        {
            continue;
        }

        // Bit index in a byte
        unsigned char bit_index = current_region->next_free_page % (sizeof(char) * 8);
        unsigned long bitmap_index = bit_index_to_byte_index(current_region->next_free_page);

        turn_bit_off(&current_region->bitmap[bitmap_index], bit_index);

        void *free_page_address = (void *)
                                  (current_region->base_address +
                                          (PAGE_SIZE * (current_region->next_free_page)));
        set_next_free_page_index(current_region->bitmap,
                                 current_region->bitmap_size,
                                 (unsigned long*)&current_region->next_free_page);
        return free_page_address;
    }

    // :-(
    return NULL;
}

unsigned int address_to_region(unsigned long address)
{
    for(int i = 0; i < number_of_regions; i++)
    {
        unsigned long base = physical_memory_regions[i].base_address;
        unsigned long end = base + physical_memory_regions[i].size;

        if(address >= base && address < end)
        {
            return i;
        }
    }

    return -1;
}

void free_physical_page(void* page_address)
{
    //Not aligned?
    if(PAGE_BOUNDARY(page_address) != (unsigned long)page_address)
    {
        kernel_panic();
    }

    unsigned int region_index = address_to_region((unsigned long)page_address);
    if(region_index == -1)
    {
        // :-(
        kernel_panic();
    }

    unsigned long bit_index = address_to_bit_index(page_address,
                                                   (void *) physical_memory_regions[region_index].base_address);

    unsigned long bit_location = bit_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(bit_index);

    turn_bit_on(&physical_memory_regions[region_index].bitmap[bitmap_index], bit_location);

    //Check bit index is lower than the current free page
    if (bit_index < physical_memory_regions[region_index].next_free_page)
    {
        physical_memory_regions[region_index].next_free_page = bit_index;
    }
}
