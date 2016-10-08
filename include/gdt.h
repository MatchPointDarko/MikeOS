#ifndef MIKE_OS_GDT_H
#define MIKE_OS_GDT_H
#include <common.h>

#define KERNEL_CODE_SEGMENT (0x08)
#define KERNEL_DATA_SEGMENT (0x10)

#define GDT__ON_BIT (1 << 4)
#define GDT__PRESENT (1 << 7 | GDT__ON_BIT)
#define GDT__RING3 (3 << 5 | GDT__ON_BIT)
#define GDT__RING0 (0 | GDT__ON_BIT)
#define GDT__EXECUTABLE (1 << 3 | GDT__ON_BIT)
#define GDT__READABLE_WRITEABLE (1 << 1 | GDT__ON_BIT)

void load_gdt();
void set_gdt_gate(uint32_t num, uint32_t base, uint32_t limit,
						 uint8_t access, uint8_t gran);
void remap_gdt();

#endif //MIKE_OS_GDT_H
