#ifndef MIKE_OS_COMMON_H
#define MIKE_OS_COMMON_H

#include "external_linker_symbols.h"
#define PAGE_SIZE 4096

#define NEXT_PAGE_BOUNDARY(val) ((val & ~(PAGE_SIZE - 1)) + PAGE_SIZE)
#define NULL (void*)0

typedef int size_t;

typedef enum bool {false, true} bool_t;

#endif //MIKE_OS_COMMON_H
