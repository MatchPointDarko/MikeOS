/*
 * MikeOS: Kernel heap page table manager.
 */

#include "common.h"
#include "paging.h"
#include "memory.h"
#include "physical_mm_manager.h"
#include "kheap.h"
#include "bitmap_manipulation.h"

#define KHEAP_BITMAP_SIZE (32768 * 4) //4MB
#define KHEAP_BEGIN_ADDRESS (KERNEL_VIRTUAL_OFFSET + (2 * 1024 * 1024 * 4))
#define KHEAP_END_ADDRESS ((KHEAP_BEGIN_ADDRESS) + (1024 * 1024 * 4))
#define KHEAP_RECURSIVE_ADDRESS (RECURSIVE_PGD_ADDRESS +\
                                 ADDRESS_TO_DIRECTORY_ENTRY(KHEAP_BEGIN_ADDRESS) * PAGE_SIZE)

static inline void tlb_flush();
static inline void edit_entry_attributes(void* entry, attributes_t attributes);

struct kheap_status
{
    uint8_t bitmap[KHEAP_BITMAP_SIZE];

} kheap_st = {{0,}};

//4MB of page tables
ALLOC_PAGE_TABLES(kheap_page_tables, 1);

static inline void tlb_flush()
{
    asm("invlpg [0]");
}

static inline void edit_entry_attributes(void* entry, attributes_t attributes)
{
    if(entry != NULL)
    {
        *(uint32_t*)entry |= attributes;
    }
}

static void map_physical_pages(void* address, uint32_t num_pages)
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
    map_kheap_to_pgd();

    //We still depend on the identity mapping.
    memset(0xFF, kheap_st.bitmap, sizeof(kheap_page_tables));
}

void map_kheap_to_pgd()
{
    page_table_ptr_t* pgd = (page_table_ptr_t*)RECURSIVE_PGT_ADDRESS;
    uint32_t kheap_pgd_entry = ADDRESS_TO_DIRECTORY_ENTRY(KHEAP_BEGIN_ADDRESS);

    pgd[kheap_pgd_entry] = (page_table_ptr_t)kheap_page_tables;

    edit_entry_attributes(&pgd[kheap_pgd_entry], READ_WRITE | PRESENT);
}

void* alloc_kheap_pages(uint32_t num_pages)
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

                //Allocate physical pages
                map_physical_pages(address, num_pages);

                //update the bitmap.
                turn_bits_off(&kheap_st.bitmap[i], index, num_pages);
                break;
            }
        }
    }

    return address;
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

