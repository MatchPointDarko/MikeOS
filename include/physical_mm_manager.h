#ifndef MIKE_OS_PHYSICAL_MM_MANAGER_H
#define MIKE_OS_PHYSICAL_MM_MANAGER_H

#include "multiboot_info.h"

/* Function: phy_memory_manager_init
 * ------------------
 * Initialize the physical memory manager.
 *
 * map_addr: a linked list of the available ram regions.
 * map_length: size of the linked list.
 */
void phy_memory_manager_init(multiboot_memory_map_t* map_addr, unsigned int map_length);

/* Function: allocate_physical_page
 * ------------------
 * Allocate a physical page(4KB)
 *
 * returns: a pointer to the physical page.
 */
void* allocate_physical_page();

/* Function: free_physical_page.
 * ------------------
 * Free a used physical page.
 *
 * page_addr: a pointer to the used physical page.
 */
void free_physical_page(void* page_addr);

#endif //MIKE_OS_PHYSICAL_MM_MANAGER_H
