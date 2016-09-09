#ifndef MIKE_OS_COMMON_H
#define MIKE_OS_COMMON_H

#include "common_constants.h"
#define PAGE_SIZE 4096

#define NEXT_PAGE_BOUNDARY(val) (((unsigned long)val & ~(PAGE_SIZE - 1)) + PAGE_SIZE)
#define PAGE_BOUNDARY(val) ((unsigned long)val & ~(PAGE_SIZE - 1))
#define NULL (void*)0

typedef int size_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;

typedef enum bool {false, true} bool_t;

#define HLT() while(true) {asm("hlt");}

#endif //MIKE_OS_COMMON_H
