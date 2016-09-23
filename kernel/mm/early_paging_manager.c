/*
 * MikeOS: Virtual memory and paging manager.
 * TODO: Kernel virtual memory manager needs refactoring asap!
 */

#include "common.h"
#include "memory.h"
#include "bitmap_manipulation.h"
#include "physical_mm_manager.h"
#include "common_constants.h"
#include "paging.h"

#define KERNEL_PGD_ENTRY_INDEX 768
#define NUM_KERNEL_PAGE_TABLES (2)

page_table_ptr_t init_page_directory[PAGE_DIRECTORY_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables"))) = {0};

//8MB of page tables
ALLOC_PAGE_TABLES(kernel_page_tables, NUM_KERNEL_PAGE_TABLES);

static inline void __attribute__((section(".init"))) edit_entry_attributes(void* entry, attributes_t attributes)
{
    if(entry != NULL)
    {
        *(uint32_t*)entry |= attributes;
    }
}

/*
 * Create the page tables that point to the first 8MB(BIOS, and kernel)
 */

static void __attribute__((section(".init"))) create_kernel_page_tables()
{
    uint32_t current_page = 0;
    for(int i = 0; i < NUM_KERNEL_PAGE_TABLES; i++)
    {
        physical_page_ptr_t current_pgt = (physical_page_ptr_t)GET_PAGE_TABLE(kernel_page_tables, i);
        for(int j = 0; j < PAGE_TABLE_SIZE; j++, current_page += PAGE_SIZE)
        {
            current_pgt[j] = current_page;
            //edit_entry_attributes(&current_pgt[j], READ_WRITE | PRESENT);
            current_pgt[j] |= 0x3;
        }
    }

}

/*
 * Identity map the first 4MB to themselves.
 */
__attribute__((section(".init"))) static void map_lower_mem()
{
    for(int i = 0; i < NUM_KERNEL_PAGE_TABLES; i++)
    {
        init_page_directory[i] = (page_table_ptr_t)GET_PAGE_TABLE(kernel_page_tables, i);
        edit_entry_attributes(&init_page_directory[i], READ_WRITE | PRESENT);
    }
}

/*
 * Map the kernel page table, 0xc0000000 + 4MB
 */
__attribute__((section(".init"))) static void map_kernel_memory()
{
    for(int i = KERNEL_PGD_ENTRY_INDEX; i < KERNEL_PGD_ENTRY_INDEX + NUM_KERNEL_PAGE_TABLES; i++)
    {
        init_page_directory[i] = (page_table_ptr_t)GET_PAGE_TABLE(kernel_page_tables, i - KERNEL_PGD_ENTRY_INDEX);
        edit_entry_attributes(&init_page_directory[i], READ_WRITE | PRESENT);
    }
}

__attribute__((section(".init"))) static inline void enable_paging()
{
    //Update page directory register
    asm("lea eax, [init_page_directory]");
    asm("mov cr3, eax");

    //Turn paging bit on
    asm("mov eax, cr0");
    asm("or eax, 0x80000000");
    asm("mov cr0, eax");
}
__attribute__((section(".init"))) void init_recursive_page_directory()
{
    init_page_directory[PAGE_DIRECTORY_SIZE - 1] = (page_table_ptr_t)init_page_directory;
    edit_entry_attributes(&init_page_directory[PAGE_DIRECTORY_SIZE - 1], READ_WRITE | PRESENT);
}

__attribute__((section(".init"))) void paging_init()
{
    create_kernel_page_tables();
    map_lower_mem();

    map_kernel_memory();
    init_recursive_page_directory();
    enable_paging();
}
