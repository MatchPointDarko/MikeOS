/*
 * MikeOS: GDT Initializing.
 */

#include <libc/stdio.h>
#include <logging/logger.h>
#include <gdt.h>

#define GDT__ON_BIT (1 << 4)
#define GDT__PRESENT (1 << 7 | GDT__ON_BIT)
#define GDT__RING3 (3 << 5 | GDT__ON_BIT)
#define GDT__RING0 (0 | GDT__ON_BIT)
#define GDT__EXECUTABLE (1 << 3 | GDT__ON_BIT)
#define GDT__READABLE_WRITEABLE (1 << 1 | GDT__ON_BIT)

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

__attribute__((section(".tables"))) struct gdt_entry early_gdt[6];

__attribute__((section(".tables"))) struct gdt_ptr gp;

static struct gdt_entry* gdt = (struct gdt_entry*)((uint32_t)early_gdt + KERNEL_VIRTUAL_OFFSET);

__attribute__((section(".init")))
static void early_set_gdt_gate(int num, unsigned long base, unsigned long limit,
						 		unsigned char access, unsigned char gran)
{
	early_gdt[num].base_low = (base & 0xFFFF);
	early_gdt[num].base_middle = (base >> 16) & 0xFF;
	early_gdt[num].base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	early_gdt[num].limit_low = (limit & 0xFFFF);
	early_gdt[num].granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	early_gdt[num].granularity |= (gran & 0xF0);
	early_gdt[num].access = access;
}

__attribute__((section(".init"))) void load_gdt()
{
	//Set up the pointer.
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = (unsigned int)early_gdt;

	//NULL Descriptor
	early_set_gdt_gate(0, 0, 0, 0, 0);

	//Code
	early_set_gdt_gate(1, 0, 0xFFFFFFFF,
					   GDT__PRESENT | GDT__RING0 | GDT__READABLE_WRITEABLE | GDT__EXECUTABLE, 0xCF);

	//Data
	early_set_gdt_gate(2, 0, 0xFFFFFFFF, GDT__PRESENT | GDT__RING0 | GDT__READABLE_WRITEABLE, 0xCF);

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

	gp.base += KERNEL_VIRTUAL_OFFSET;

	asm("lea eax, [gp]");
	asm("add eax, 0xc0000000");
	asm("lgdt [eax]");

	log_print(LOG_DEBUG, "Remapped GDT successfuly!");
}

void set_gdt_gate(uint32_t num, uint32_t base, uint32_t limit,
				  uint8_t access, uint8_t gran)
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

