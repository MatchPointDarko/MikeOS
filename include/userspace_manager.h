#ifndef MIKE_OS_USERSPACE_MANAGER_H
#define MIKE_OS_USERSPACE_MANAGER_H

#include <common.h>

void userspace_init();
void set_kernel_stack(uint32_t address);
void jmp_to_userspace(uint32_t eip, uint32_t esp);

#endif //MIKE_OS_USERSPACE_MANAGER_H
