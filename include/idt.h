//
// Created by sourcer on 03/06/16.
//

#ifndef __IDT__
#define __IDT__

void idt_init();
void load_idt(void*);
void load_idt_entry(char isr_number, unsigned long base, short int selector, char flags);

#endif
