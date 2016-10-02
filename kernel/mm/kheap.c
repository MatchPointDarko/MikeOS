/*
 * MikeOS: Kernel heap page table manager.
 */

#include <common.h>
#include <libc/memory.h>
#include <mm/paging.h>
#include <mm/physical_mm_manager.h>
#include <mm/kheap.h>
#include <mm/bitmap_manipulation.h>
#include <mm/virtual_mm_manager.h>

#define KHEAP_BITMAP_SIZE (32768 * 4) //4MB
#define KHEAP_BEGIN_ADDRESS (KERNEL_VIRTUAL_OFFSET + (2 * 1024 * 1024 * 4))
#define KHEAP_END_ADDRESS ((KHEAP_BEGIN_ADDRESS) + (1024 * 1024 * 4))
#define KHEAP_RECURSIVE_ADDRESS (RECURSIVE_PGD_ADDRESS +\
                                 ADDRESS_TO_DIRECTORY_ENTRY(KHEAP_BEGIN_ADDRESS) * PAGE_SIZE)

struct kheap_status
{
    uint8_t bitmap[KHEAP_BITMAP_SIZE];

} kheap_st = {{0,}};

//4MB of page tables
ALLOC_PAGE_TABLES(kheap_page_tables, 1);

uint32_t kheap_get_physical_address(uint32_t virtual_address)
{
    if(!(virtual_address >= KHEAP_BEGIN_ADDRESS && virtual_address < KHEAP_END_ADDRESS))
    {
        return 0;
    }

    if(PAGE_BOUNDARY(virtual_address) != (uint32_t)virtual_address)
    {
        return INVALID_ARGUMENT;
    }

    physical_page_ptr_t* base_page_table = (physical_page_ptr_t*)KHEAP_RECURSIVE_ADDRESS;

    uint32_t address = (uint32_t)base_page_table[ADDRESS_TO_PAGE_TABLE_ENTRY(virtual_address)];

    //Delete the paging related info bits.
    return ((address >> 8) << 8);
}

static void alloc_and_map_physical_pages(void* address, uint32_t num_pages)
{
    physical_page_ptr_t* base_page_table = (physical_page_ptr_t*)KHEAP_RECURSIVE_ADDRESS;

    for(int i = ADDRESS_TO_PAGE_TABLE_ENTRY(address);
        i < ADDRESS_TO_PAGE_TABLE_ENTRY(address) + num_pages;
        i++)
    {
        base_page_table[i] = allocate_physical_page();
        edit_entry_attributes(&base_page_table[i], READ_WRITE | PRESENT);
    }
}

static void parse_lookup_table(uint8_t* lookup_table, uint8_t* bitmap, uint32_t num_bytes)
{
    //Assumes the lookup table is in the correct size, and no overflow will happen.
    for(int i = 0; i < num_bytes * 8; i++)
    {
        lookup_table[i] = extract_bit(bitmap[(i / 8)], i % 8);
    }
}

static int32_t find_continuous_blocks_index(uint8_t* buffer, uint32_t buffer_size, uint32_t num_blocks)
{
    int32_t index = -1;
    int32_t current_num_blocks = num_blocks;

    for(int i = 0; i < buffer_size; i++)
    {
        if(num_blocks == 0)
        {
            return index;
        }

        if(buffer[i] == PAGE_AVAILABLE)
        {
            if(index == -1)
                index = i;
            num_blocks--;
        }
        else
        {
            index = -1;
        }
    }
}

void kheap_init()
{
    map_kheap_to_pgd((page_table_ptr_t)RECURSIVE_PGT_ADDRESS);

    //We still depend on the identity mapping.
    memset(0xFF, kheap_st.bitmap, sizeof(kheap_page_tables));
}


static void* get_free_continuous_blocks_address(uint32_t num_pages)
{
    uint32_t num_bytes = num_pages / 8;
    if(num_pages % 8 != 0)
    {
        num_bytes++;
    }

    uint32_t num_bytes_itr = num_bytes;
    uint8_t lookup_table[num_bytes * 8];
    uint32_t index = 0;
    void* address = NULL;

    for(int i = 0; i < KHEAP_BITMAP_SIZE - num_bytes - 1; i++, num_bytes_itr = num_bytes)
    {
        if(kheap_st.bitmap[i] != 0)
        {
            //Worse case scenario, the lookup table takes 1MB.
            //But a much cleaner and readable code has been achieved by that.
            parse_lookup_table(lookup_table, &kheap_st.bitmap[i], num_bytes);

            //Search for num_pages free bytes.
            index = find_continuous_blocks_index(lookup_table, num_bytes * 8, num_pages);
            if(index != -1)
            {
                address = (void*)(KHEAP_BEGIN_ADDRESS + (((i * 8) + index) * PAGE_SIZE));

                //update the bitmap.
                turn_bits_off(&kheap_st.bitmap[i], index, num_pages);
                break;
            }
        }
    }

    return address;
}

void* alloc_kheap_pages(uint32_t num_pages)
{
    void* address = get_free_continuous_blocks_address(num_pages);

    alloc_and_map_physical_pages(address, num_pages);

    return address;
}

void* map_physical_to_kheap(uint32_t physical_address, uint32_t num_pages)
{
    void* free_virtual_address = get_free_continuous_blocks_address(num_pages);
    physical_page_ptr_t* base_page_table = (physical_page_ptr_t*)KHEAP_RECURSIVE_ADDRESS;

    for(int i = ADDRESS_TO_PAGE_TABLE_ENTRY(free_virtual_address);
        i < ADDRESS_TO_PAGE_TABLE_ENTRY(free_virtual_address) + num_pages;
        i++, physical_address += PAGE_SIZE)
    {
        base_page_table[i] = (physical_page_ptr_t)physical_address;
        edit_entry_attributes(&base_page_table[i], READ_WRITE | PRESENT);
    }

    return free_virtual_address;
}

error_code_t free_kheap_pages(void* address, uint32_t num_pages)
{
    if(!((uint32_t)address >= KHEAP_BEGIN_ADDRESS
        && (uint32_t)address < KHEAP_END_ADDRESS))
    {
        return INVALID_ARGUMENT;
    }

    if(PAGE_BOUNDARY(address) != (uint32_t)address)
    {
        return INVALID_ARGUMENT;
    }

    //Update bitmap
    uint32_t bit_index = address_to_bit_index((uint32_t)address, KHEAP_BEGIN_ADDRESS);

    turn_bits_on(kheap_st.bitmap, bit_index, num_pages);

    //Free the actual page in the page table.
    physical_page_ptr_t* base_page_table = (physical_page_ptr_t*)KHEAP_RECURSIVE_ADDRESS;

    for(int i = ADDRESS_TO_PAGE_TABLE_ENTRY(address);
        i < ADDRESS_TO_PAGE_TABLE_ENTRY(address) + num_pages;
        i++)
    {
        free_physical_page((uint32_t)base_page_table[i]);
        base_page_table[i] = 0;
    }

    tlb_flush();

    return SUCCESS;
}


error_code_t map_kheap_to_pgd(page_table_ptr_t page_directory)
{
    if(page_directory == NULL)
    {
        return INVALID_ARGUMENT;
    }

    uint32_t kheap_pgd_entry = ADDRESS_TO_DIRECTORY_ENTRY(KHEAP_BEGIN_ADDRESS);

    page_directory[kheap_pgd_entry] = (physical_page_ptr_t)kheap_page_tables;

    edit_entry_attributes(&page_directory[kheap_pgd_entry], READ_WRITE | PRESENT);
}
