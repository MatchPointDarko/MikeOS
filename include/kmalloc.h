#ifndef MIKE_OS_KMALLOC_H
#define MIKE_OS_KMALLOC_H

#include "common.h"

/* Function: kmalloc
 * ------------------
 * Allocate a memory block from the kernel heap.
 *
 * size: the size of the memory block requested.
 *
 * returns: a pointer to the memory block requested.
 */
void* kmalloc(size_t size);

/* Function: kfree.
 * ------------------
 * Free a used kernel heap memory block.
 *
 * address: the address to the memory block.
 *
 * NOTE: Does not yet assume the address is aligned and valid.
 */
void kfree(void* address);

#endif //MIKE_OS_KMALLOC_H
