//
// Created by sourcer on 21/06/16.
//

#ifndef MIKE_OS_PHYSICAL_MM_MANAGER_H
#define MIKE_OS_PHYSICAL_MM_MANAGER_H
#include "multiboot_info.h"

void memory_manager_init(multiboot_memory_map_t* map_addr, unsigned int map_length);
void* allocate_physical_page();
void free_physical_page(void* page_addr);

#endif //MIKE_OS_PHYSICAL_MM_MANAGER_H
