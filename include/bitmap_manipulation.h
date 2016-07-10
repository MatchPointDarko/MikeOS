//
// Created by sourcer on 10/07/16.
//

#ifndef MIKE_OS_BITMAP_MANIPULATION_H
#define MIKE_OS_BITMAP_MANIPULATION_H

#define MANAGED_MEMORY_PAGE_ALIGN(value, addr) (PAGE_ALIGN(value)) + \
                                               (addr - \
                                               (PAGE_ALIGN(addr)))

unsigned long bit_index_to_byte_index(unsigned long bit_index);
unsigned long address_to_bit_index(void* addr, void* start_address);
void set_next_free_page_index(char* bitmap, unsigned long* current_free_page_index);

#endif //MIKE_OS_BITMAP_MANIPULATION_H
