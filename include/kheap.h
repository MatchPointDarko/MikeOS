#ifndef MIKE_OS_KHEAP_H
#define MIKE_OS_KHEAP_H

#include "common.h"
#include "error_codes.h"

void* alloc_kheap_pages(uint32_t num_pages);
error_code_t free_kheap_pages(void* address, uint32_t num_pages);
void map_kheap_to_pgd();
void kheap_init();

#endif //MIKE_OS_KHEAP_H
