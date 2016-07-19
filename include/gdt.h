#ifndef MIKE_OS_GDT_H
#define MIKE_OS_GDT_H

void load_gdt();
void set_gdt_gate(int num, unsigned long base, unsigned long limit,
						 unsigned char access, unsigned char gran);
void remap_gdt();

#endif //MIKE_OS_GDT_H
