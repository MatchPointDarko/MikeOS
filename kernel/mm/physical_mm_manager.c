/*
 * MikeOS: Physical memory manager.
 * NOTE: The implementation is in the approach of KISS(Keep it simple stupid)
 * I didn't want to make this complicated at first, because it fulfills its purpose quite well.
 * Maybe a refactoring will occur in the future.
 */

#include <logger.h>
#include <panic.h>
#include <stdio.h>
#include <memory.h>
#include <logger.h>
#include <bitmap_manipulation.h>
#include <physical_mm_manager.h>
#include <error_codes.h>
#include <paging.h>

#define ADDRESS_SPACE_SIZE (0x100000000) // 4GB
#define BITMAP_SIZE ((ADDRESS_SPACE_SIZE) / (PAGE_SIZE) / 8)

typedef struct physical_mem
{
    uint8_t bitmap[BITMAP_SIZE];
    bool_t no_page_available;

} physical_mem_t;

static error_code_t change_pages_state(uint32_t begin, uint32_t end, page_state_t state);
static error_code_t change_page_state(uint32_t page_address, page_state_t state);

static physical_mem_t physical_memory = {{0, }, false};

static error_code_t change_page_state(uint32_t page_address, page_state_t state)
{
    if(PAGE_BOUNDARY(page_address) != page_address)
    {
        return INVALID_ARGUMENT;
    }

    uint32_t bit_index = address_to_bit_index(page_address, 0);
    uint32_t byte_index = bit_index / 8;

    if(state == PAGE_UNAVAILABLE)
    {
        turn_bit_off(&physical_memory.bitmap[byte_index], bit_index % 8);
    }

    else if(state == PAGE_AVAILABLE)
    {
        turn_bit_on(&physical_memory.bitmap[byte_index], bit_index % 8);
    }

    return SUCCESS;
}

static error_code_t change_pages_state(uint32_t begin, uint32_t end, page_state_t state)
{
    if(PAGE_BOUNDARY(begin) != begin || PAGE_BOUNDARY(end) != end)
    {
        return INVALID_ARGUMENT;
    }

    if(end < begin)
    {
        return INVALID_ARGUMENT;
    }

    for(uint32_t address = begin; address < end; address += PAGE_SIZE)
    {
        if(change_page_state(address, state) != SUCCESS)
        {
            //Undefined behavior, shouldn't happen.
            return FAILURE;
        }
    }

    return SUCCESS;
}

/* Map the RAM, using the GRUB memory map. */
void map_memory(multiboot_memory_map_t* map_addr, unsigned int map_length)
{
    log_print(LOG_DEBUG, "Starting to map the RAM");
    for(multiboot_memory_map_t* map_iterator = map_addr;
        map_iterator < (map_addr + map_length);
        map_iterator = (multiboot_memory_map_t*)((unsigned int)map_iterator + map_iterator->size + 4))
    {
        if(map_iterator->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            log_print(LOG_DEBUG, "Address: %x", map_iterator->addr);
            log_print(LOG_DEBUG, "Length: %x", map_iterator->len);

            uint32_t address = map_iterator->addr;
            uint32_t len = map_iterator->len;

            if(PAGE_BOUNDARY(address) != address)
            {
                address = NEXT_PAGE_BOUNDARY(address);
                if(len <= PAGE_SIZE)
                {
                    continue;
                }

                len -= (address - map_iterator->addr);
            }

            if(PAGE_BOUNDARY(len) != len)
            {
                if(PAGE_BOUNDARY(len) < len)
                {
                    len = PAGE_BOUNDARY(len);
                }

                else
                {
                    len = PAGE_BOUNDARY(len) - PAGE_SIZE;
                }
            }

            if(change_pages_state(address, address + len, PAGE_AVAILABLE)
                        != SUCCESS)
            {
                kernel_panic();
            }
        }
    }
}

void phy_memory_manager_init(multiboot_memory_map_t* map_addr, unsigned int map_length)
{
    map_memory(map_addr, map_length);

    //Low 1MB
    change_pages_state(0, 0xff000, PAGE_UNAVAILABLE);

    //Kernel
    change_pages_state((uint32_t)&kernel_start, (uint32_t)&kernel_end, PAGE_UNAVAILABLE);
}

/* Allocate a single page. */
void* allocate_physical_page()
{
    if(physical_memory.no_page_available)
    {
        return NULL;
    }

    //Locate an available memory page
    //Skip the lower 1MB and Kernel
    for(uint32_t i = ((uint32_t)&kernel_end) / PAGE_SIZE / (sizeof(uint8_t) * 8);
        i < BITMAP_SIZE;
        ++i)
    {
        if(physical_memory.bitmap[i] != 0)
        {
            uint32_t bit_index_in_bitmap = (i * 8) + get_msb_index(physical_memory.bitmap[i]);
            uint32_t address = bit_index_in_bitmap * PAGE_SIZE;

            change_page_state(address, PAGE_UNAVAILABLE);

            return (void*)address;
        }
    }

    physical_memory.no_page_available = true;
    return NULL;
}

error_code_t free_physical_page(uint32_t page_address)
{
    error_code_t error_code = SUCCESS;

    if((error_code = change_page_state(page_address, PAGE_AVAILABLE) == SUCCESS))
    {
        if(physical_memory.no_page_available)
        {
            physical_memory.no_page_available = false;
        }
    }

    return error_code;
}

error_code_t register_ramdisk(uint32_t start_address, uint32_t end_address)
{
    return change_pages_state(start_address, end_address, PAGE_UNAVAILABLE);
}

error_code_t unregister_ramdisk(uint32_t start_address, uint32_t end_address)
{
    return change_pages_state(start_address, end_address, PAGE_AVAILABLE);
}
