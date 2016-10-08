#ifndef __IDT__
#define __IDT__

#define IDT__RING3 (3 << 5)
#define IDT__RING0 (0)
#define IDT__PRESENT (1 << 7)
#define IDT__INTERRUPT_GATE (0x0e)
#define IDT__EXCEPTION_GATE (0x0F)

void idt_init();
void load_idt(void*);
void load_idt_entry(unsigned char isr_number, unsigned long base, short int selector, char flags);

#endif
