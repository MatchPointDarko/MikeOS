/*
 * MikeOS: Virtual memory manager.
 * Everything that needs to be done on page directories is implemented here.
 * (kheap, early paging are exceptions)
 */

#include <kmalloc.h>
#include <common.h>
#include <error_codes.h>
#include <kheap.h>
#include <memory.h>
#include <virtual_mm_manager.h>
#include <physical_mm_manager.h>

#define USER_SPACE_LIMIT (767)

extern physical_page_ptr_t kernel_page_tables[NUM_KERNEL_PAGE_TABLES][PAGE_TABLE_SIZE];

static error_code_t free_user_virtual_page(uint32_t virtual_address);

void tlb_flush()
{
    asm("invlpg [0]");
}

void edit_entry_attributes(void* entry, attributes_t attributes)
{
    if(entry != NULL)
    {
        *(uint32_t*)entry |= attributes;
    }
}
/*
 * * Allocate a page directory,
 * * Copy the kernel page tables to it.
 * * Make it recursive page directory.
 */
static error_code_t alloc_page_dir(virtual_address_space_t* vmm)
{
    page_table_ptr_t page_directory = alloc_kheap_pages(1);

    if(page_directory == NULL)
    {
        return FAILURE;
    }

    //All of the pointers are NULL
    memset(NULL, (char *)page_directory, PAGE_SIZE);

    //Copy kernel to page directory
    for(int i = KERNEL_PGD_ENTRY_INDEX; i < KERNEL_PGD_ENTRY_INDEX + NUM_KERNEL_PAGE_TABLES; i++)
    {
        page_directory[i] = (physical_page_ptr_t)GET_PAGE_TABLE(kernel_page_tables, i - KERNEL_PGD_ENTRY_INDEX);
        edit_entry_attributes(&page_directory[i], READ_WRITE | PRESENT);
    }

    //Copy kheap to the page directory
    map_kheap_to_pgd(page_directory);

    vmm->page_directory = page_directory;
    vmm->physical_pgd_address = kheap_get_physical_address((uint32_t)page_directory);

    //Make it recursive page directory
    vmm->page_directory[PAGE_DIRECTORY_SIZE - 1] = (physical_page_ptr_t)vmm->physical_pgd_address;
    edit_entry_attributes(&vmm->page_directory[PAGE_DIRECTORY_SIZE - 1], READ_WRITE | PRESENT);

    return SUCCESS;
}

virtual_address_space_t* alloc_virtual_address_space()
{
    virtual_address_space_t* vmm = kmalloc(sizeof(virtual_address_space_t));

    if(vmm == NULL)
    {
        return NULL;
    }

    if(alloc_page_dir(vmm) != SUCCESS)
    {
        kfree(vmm);
        return NULL;
    }

    return vmm;
}

/*
 * Assumes the page directory is in CR3.
 */
error_code_t free_virtual_address_space(virtual_address_space_t* vmm)
{
    if(vmm == NULL)
    {
        return INVALID_ARGUMENT;
    }

    page_table_ptr_t page_directory = (page_table_ptr_t)RECURSIVE_PGD_ADDRESS;
    page_table_ptr_t page_directory_ptr = (page_table_ptr_t)RECURSIVE_PGT_ADDRESS;

    for(int i = 0; i < USER_SPACE_LIMIT; i++)
    {
        if(vmm->page_directory[i] != NULL)
        {
            //Iterate on the page table.
            for(int j = 0; j < PAGE_TABLE_SIZE; j++)
            {
                if(page_directory[i * 1024 + j] != NULL)
                {
                    free_physical_page((uint32_t)page_directory[i * 1024 + j]);
                }
            }

            free_physical_page((uint32_t)vmm->page_directory[i]);
        }
    }

    free_kheap_pages(vmm->page_directory, 1);
    kfree(vmm);

    return SUCCESS;
}

static error_code_t free_user_virtual_page(uint32_t virtual_address)
{
    uint32_t pgd_index = ADDRESS_TO_DIRECTORY_ENTRY(virtual_address);
    page_table_ptr_t page_directory_ptr = (page_table_ptr_t)RECURSIVE_PGT_ADDRESS;

    free_physical_page((uint32_t)page_directory_ptr[pgd_index]);
}

error_code_t switch_virtual_address_space(virtual_address_space_t* vmm)
{
    if(vmm == NULL || vmm->page_directory == NULL)
    {
        return INVALID_ARGUMENT;
    }

    asm("mov cr3, %0" : "+r"(vmm->physical_pgd_address));

    tlb_flush();

    return SUCCESS;
}

/*
 * Map a user-space virtual address to a physical address.
 */
static void map_user_virtual_address_to_physical(uint32_t virtual_address, uint32_t physical_address)
{
    page_table_ptr_t page_directory = (page_table_ptr_t)RECURSIVE_PGD_ADDRESS;
    page_table_ptr_t page_directory_ptr = (page_table_ptr_t)RECURSIVE_PGT_ADDRESS;

    uint32_t pgd_index = ADDRESS_TO_DIRECTORY_ENTRY(virtual_address);
    uint32_t pgt_index = ADDRESS_TO_PAGE_TABLE_ENTRY(virtual_address);

    if(page_directory_ptr[pgd_index] == NULL)
    {
        page_directory_ptr[pgd_index] = allocate_physical_page();
        edit_entry_attributes(&page_directory_ptr[pgd_index], READ_WRITE | PRESENT | USER);
        memset(NULL, (char *)(page_directory + (pgd_index * 1024)), PAGE_SIZE);
    }

    page_directory[pgd_index * PAGE_TABLE_SIZE + pgt_index] = (physical_page_ptr_t)physical_address;

    edit_entry_attributes(&page_directory[pgd_index * PAGE_TABLE_SIZE + pgt_index], READ_WRITE | PRESENT | USER);
}

/*
 * Will allocate and map to the CURRENT PAGE DIRECTORY, in CR3.
 * Assumes its recursive! (It is if you used the API like a normal person)
 */
error_code_t mmap(uint32_t virtual_address, uint32_t num_pages)
{
    if(virtual_address >= KERNEL_VIRTUAL_OFFSET
                || (virtual_address + num_pages * PAGE_SIZE) >= KERNEL_VIRTUAL_OFFSET)
    {
        return INVALID_ARGUMENT;
    }

    for(int i = virtual_address; i < virtual_address + (num_pages * PAGE_SIZE); i += PAGE_SIZE)
    {
        map_user_virtual_address_to_physical(i, (uint32_t)allocate_physical_page());
    }

    return SUCCESS;
}
