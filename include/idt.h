#ifndef __IDT__
#define __IDT__

#define KERNEL_CODE_SEGMENT 0x08

void idt_init();
void load_idt(void*);
void load_idt_entry(unsigned char isr_number, unsigned long base, short int selector, char flags);

#endif
