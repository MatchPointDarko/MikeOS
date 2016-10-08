/*
 * MikeOS: Userspace manager.
 */

#include <gdt.h>
#include <common.h>

#define TSS_SEGMENT_ENTRY (0x5)

#define RING3_DATA_SEGMENT_ENTRY (0x4)
#define RING3_DATA_FLAG (0xF2)

#define RING3_CODE_SEGMENT_ENTRY (0x3)
#define RING3_CODE_FLAG (0xFA)

typedef struct tss_struct
{
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;        // The stack segment to load when we change to kernel mode.
    uint32_t esp1;       // everything below here is unusued now..
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} tss_struct_t;

tss_struct_t task_state __attribute__((aligned(PAGE_SIZE))) = {0, };

static void load_task_state_segment()
{
    //TSS
    set_gdt_gate(TSS_SEGMENT_ENTRY, (uint32_t)&task_state,
                 ((uint32_t)&task_state) + sizeof(tss_struct_t) - 1, 0xe9, 0x00);

    task_state.ss0 = 0x10;
    asm("mov ax, 0x28");
    asm("ltr ax");
}

static void load_ring3_segments()
{
    //Ring 3 code segment
    set_gdt_gate(RING3_CODE_SEGMENT_ENTRY, 0, 0xFFFFFFFF,
                 GDT__PRESENT | GDT__RING3 | GDT__READABLE_WRITEABLE | GDT__EXECUTABLE, 0xCF);

    //Ring 3 data segment
    set_gdt_gate(RING3_DATA_SEGMENT_ENTRY, 0, 0xFFFFFFFF,
                 GDT__PRESENT | GDT__RING3 | GDT__READABLE_WRITEABLE, 0xCF);

    load_task_state_segment();
}

void userspace_init()
{
    load_ring3_segments();
}

void set_kernel_stack(uint32_t address)
{
    task_state.esp0 = address;
}
