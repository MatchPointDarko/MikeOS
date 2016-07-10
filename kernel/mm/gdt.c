#include "stdio.h"
#include "logger.h"

struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* Our GDT, with 3 entries, and finally our special GDT pointer */
__attribute__((section(".tables"))) struct gdt_entry gdt[3];

__attribute__((section(".tables"))) struct gdt_ptr gp;

__attribute__((section(".init"))) static void set_gdt_gate(int num, unsigned long base, unsigned long limit,
														   unsigned char access, unsigned char gran)
{
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

__attribute__((section(".init"))) void load_gdt()
{
	//Set up the pointer.
	gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
	gp.base = (unsigned int)&gdt;

	//NULL Descriptor, legacy
	set_gdt_gate(0, 0, 0, 0, 0);

	//Code
	set_gdt_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	//Data
	set_gdt_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	asm("lgdt [gp]");
	asm("mov ax, 0x10");
	asm("mov ds, ax");
	asm("mov es, ax");
	asm("mov fs, ax");
	asm("mov gs, ax");
	asm("mov ss, ax");
	asm("jmp 0x08:foo_label");
	//Load code segment
	asm("foo_label:");
}

/* Remap GDT to its virtual form */
void remap_gdt()
{
	log_print(LOG_DEBUG, "Remaping GDT");
	asm("lgdt [gp + 0xC0000000]");
	log_print(LOG_DEBUG, "Remapped GDT successfuly!");
}
