/*
 * MikeOS: Task implementation. (similar to linux's task_struct)
 */
#include "gdt.h"
#include "common.h"

#define USER_DATA_SEGMENT_ENTRY (0x3)
#define USER_CODE_SEGMENT_ENTRY (0x4)
#define TSS_SEGMENT_ENTRY (0x5)

#define PUSH_USER_SPACE_DATA_SEGMENT
#define PUSH_USER_SPACE_CODE_SEGMENT

typedef struct tss_struct
{
    unsigned long prev_tss;
    unsigned long esp0;
    unsigned long ss0;        // The stack segment to load when we change to kernel mode.
    unsigned long esp1;       // everything below here is unusued now..
    unsigned long ss1;
    unsigned long esp2;
    unsigned long ss2;
    unsigned long cr3;
    unsigned long eip;
    unsigned long eflags;
    unsigned long eax;
    unsigned long ecx;
    unsigned long edx;
    unsigned long ebx;
    unsigned long esp;
    unsigned long ebp;
    unsigned long esi;
    unsigned long edi;
    unsigned long es;
    unsigned long cs;
    unsigned long ss;
    unsigned long ds;
    unsigned long fs;
    unsigned long gs;
    unsigned long ldt;
    unsigned short trap;
    unsigned short iomap_base;
} tss_struct_t;

typedef struct task
{
    uint32_t pid;
    void* stack_begin;
    void* txt_begin;
    void* data_begin;
    tss_struct_t arch_task_info;
} task_t;

/* Load all user space entries to GDT.
 * NOTE: calls 'set_gdt_gate' which is relocated at 'init' section,
 * i.e. a physical address. TODO: maybe add an offset of virtual addr to
 * the function pointer, or rely on identity mapping.
 */

static void load_task_state_segment()
{
    //TSS
    set_gdt_gate(TSS_SEGMENT_ENTRY, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    asm("mov eax, 0x5");
    asm("ltr [eax]");
}

void load_user_space_entries()
{
    //Ring 3 code segment
    set_gdt_gate(USER_CODE_SEGMENT_ENTRY, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    //Ring 3 data segment
    set_gdt_gate(USER_DATA_SEGMENT_ENTRY, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    load_task_state_segment();
}

void goto_userspace(void* user_space_address)
{
    //TODO..
}
