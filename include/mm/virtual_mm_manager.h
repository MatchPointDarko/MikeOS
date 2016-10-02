#ifndef MIKE_OS_VIRTUAL_MM_MANAGER_H
#define MIKE_OS_VIRTUAL_MM_MANAGER_H

#include <mm/paging.h>
#include <error_codes.h>
#include <common.h>

typedef struct virtual_address_space
{
    page_table_ptr_t page_directory;
    uint32_t physical_pgd_address;
} virtual_address_space_t;

void edit_entry_attributes(void* entry, attributes_t attributes);
virtual_address_space_t* alloc_virtual_address_space();
void tlb_flush();
error_code_t switch_virtual_address_space(virtual_address_space_t* vmm);
error_code_t free_virtual_address_space(virtual_address_space_t* vmm);
error_code_t mmap(uint32_t virtual_address, uint32_t num_pages);

#endif //MIKE_OS_VIRTUAL_MM_MANAGER_H
