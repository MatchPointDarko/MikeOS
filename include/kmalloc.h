//
// Created by sourcer on 07/07/16.
//

#ifndef MIKE_OS_KMALLOC_H
#define MIKE_OS_KMALLOC_H

#include "common.h"

void* kmalloc(size_t size);
void kfree(void* address);

#endif //MIKE_OS_KMALLOC_H
