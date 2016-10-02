#ifndef MIKE_OS_KHEAP_H
#define MIKE_OS_KHEAP_H

#include <common.h>
#include <error_codes.h>
#include <mm/paging.h>

void* alloc_kheap_pages(uint32_t num_pages);
error_code_t free_kheap_pages(void* address, uint32_t num_pages);
error_code_t map_kheap_to_pgd(page_table_ptr_t page_directory);
void kheap_init();
uint32_t kheap_get_physical_address(uint32_t virtual_address);
void* map_physical_to_kheap(uint32_t physical_address, uint32_t num_pages);

#endif //MIKE_OS_KHEAP_H
