#ifndef MIKE_OS_PAGING_H
#define MIKE_OS_PAGING_H

#include "common_constants.h"

typedef enum page_state { PAGE_UNAVAILABLE = 0, PAGE_AVAILABLE = 1} page_state_t;

typedef uint32_t* page_table_ptr_t;
typedef uint32_t* physical_page_ptr_t;
typedef uint32_t* table_entry;

/* entry bits */
typedef enum
{
    PRESENT = 1,
    READ_WRITE,
    USER,
    WRITE_THROUGH,
    CACHE_DISABLED,
    ACCESSED,
    DIRTY,
    GLOBAL

} attributes_t;

#define PAGE_TABLE_SIZE (1024)
#define PAGE_DIRECTORY_SIZE (1024)
#define RECURSIVE_PGD_ADDRESS (0xFFC00000)
#define RECURSIVE_PGT_ADDRESS (0xFFFFF000)
#define NUM_KERNEL_PAGE_TABLES (2)
#define ALLOC_PAGE_TABLES(name, num) physical_page_ptr_t name[num][PAGE_TABLE_SIZE] \
                                     __attribute__((aligned(PAGE_SIZE)))\
                                     __attribute__((section(".tables"))) = {0}

#define GET_PAGE_TABLE(page_tables, page_table_num) page_tables[page_table_num]
#define GET_PAGE_TABLE_ENTRY(page_table, entry_num) page_table[entry_num]

#define ADDRESS_TO_DIRECTORY_ENTRY(address) ((uint32_t)address >> 22)
#define ADDRESS_TO_PAGE_TABLE_ENTRY(address) ((uint32_t)address >> 12 & 0x03ff)

#endif
