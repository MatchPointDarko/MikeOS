#ifndef MIKE_OS_BITMAP_MANIPULATION_H
#define MIKE_OS_BITMAP_MANIPULATION_H

#include <common.h>

#define NO_AVAILABLE_PAGE (-1)
#define PAGE_ALIGN(value) (((unsigned long)value) & ~(PAGE_SIZE - 1))

#define MANAGED_MEMORY_PAGE_ALIGN(value, addr) (PAGE_ALIGN(value)) + \
                                               (addr - \
                                               (PAGE_ALIGN(addr)))

unsigned long bit_index_to_byte_index(unsigned long bit_index);
unsigned long address_to_bit_index(uint32_t addr, uint32_t start_address);
void set_next_free_page_index(char* bitmap, unsigned long bitmap_size, unsigned long* current_free_page_index);

#endif //MIKE_OS_BITMAP_MANIPULATION_H
