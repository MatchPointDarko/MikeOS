/*
 * MikeOS: System call implementation.
 */

#include <idt.h>
#include <common.h>
#include <logging/logger.h>
#include <gdt.h>

#define SYSCALL_IDT_ENTRY (0x80)
#define SYSCALL_RETURN(val) (registers->eax = val)
#define jmp_to_syscall(func, registers) ((uint32_t(*)())func)()

extern void interrupt_80();
void system_call_default_handler();

static void* syscall_table[150] = {foo_syscall};

void system_calls_init()
{
    load_idt_entry(SYSCALL_IDT_ENTRY, (uint32_t)interrupt_80, KERNEL_CODE_SEGMENT,
                   IDT__PRESENT | IDT__RING3 | IDT__INTERRUPT_GATE);
    log_print(LOG_DEBUG, "Initialized system calls successfuly");
}

void default_system_call_handler(struct regs* registers)
{
    //if(!is_valid_syscall_number(registers->eax))
    {
     //  SYSCALL_RETURN(-1);
    }

    printf("lalallalalalalalalal\n");
    while(1);
    //uint32_t ret = jmp_to_syscall(syscall_table[registers->eax], registers);
    //SYSCALL_RETURN(ret);
}
