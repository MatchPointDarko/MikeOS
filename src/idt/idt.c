#include "port_io.h"
#include "keyboard_driver.h"
#include "idt.h"

#define IDT_SIZE 256
#define PIC_1_CTRL 0x20
#define PIC_2_CTRL 0xA0
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

struct idt_entry
{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char flags;
	unsigned short int offset_higherbits;
} __attribute__((packed));

struct idt_pointer
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

struct idt_entry idt_table[IDT_SIZE];
struct idt_pointer idt_ptr;

void load_idt_entry(char isr_number, unsigned long base, short int selector, char flags)
{
	idt_table[isr_number].offset_lowerbits = base & 0xFFFF;
	idt_table[isr_number].offset_higherbits = (base >> 16) & 0xFFFF;
	idt_table[isr_number].selector = selector;
	idt_table[isr_number].flags = flags;
	idt_table[isr_number].zero = 0;
}

static void initialize_idt_pointer()
{
	idt_ptr.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
	idt_ptr.base = (unsigned int)&idt_table;
}

static void initialize_pic()
{
	/* ICW1 - begin initialization */
	write_port(PIC_1_CTRL, 0x11);
	write_port(PIC_2_CTRL, 0x11);

	/* ICW2 - remap offset address of idt_table */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(PIC_1_DATA, 0x20);
	write_port(PIC_2_DATA, 0x28);

	write_port(0x21 , 0x4);
	write_port(0xA1 , 0x2);

	/* ICW3 - setup cascading */
	write_port(PIC_1_DATA, 0x00);
	write_port(PIC_2_DATA, 0x00);

	/* ICW4 - environment info */
	write_port(PIC_1_DATA, 0x01);
	write_port(PIC_2_DATA, 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);
}

void idt_init()
{
	initialize_pic();
	initialize_idt_pointer();
	load_idt(&idt_ptr);
}
