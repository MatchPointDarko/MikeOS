#ifndef MIKE_OS_PAGING_MANAGER_H
#define MIKE_OS_PAGING_MANAGER_H

void virtual_memory_manager_init();
void* allocate_kernel_virtual_page();
void free_kernel_virtual_page(void* virtual_address);

#endif //MIKE_OS_PAGING_MANAGER_H
