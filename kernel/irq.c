/*
 * MikeOS: IRQ default handler, and irq redirection.
 */

#include "port_io.h"
#include "error_codes.h"
#include "common.h"
#include "idt.h"
#include "logger.h"
#include "irq.h"

#define SLAVE_PIC_CMD (0xA0)
#define MASTER_PIC_CMD (0x20)
#define PIC_ACK (0x20)
#define NUMBER_OF_IRQS (16)

typedef struct regs
{
    uint32_t irq_number;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, cs, flags, user_esp, ss;

} regs_t;

extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();
extern inline void irq_ack(uint32_t irq_number);

static irq_handler_t irq_handlers[NUMBER_OF_IRQS] = {NULL, };

void irq_init()
{
    load_idt_entry(32, (unsigned long)irq_0, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(33, (unsigned long)irq_1, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(34, (unsigned long)irq_2, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(35, (unsigned long)irq_3, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(36, (unsigned long)irq_4, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(37, (unsigned long)irq_5, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(38, (unsigned long)irq_6, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(39, (unsigned long)irq_7, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(40, (unsigned long)irq_8, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(41, (unsigned long)irq_9, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(42, (unsigned long)irq_10, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(43, (unsigned long)irq_11, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(44, (unsigned long)irq_12, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(45, (unsigned long)irq_13, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(45, (unsigned long)irq_14, KERNEL_CODE_SEGMENT, 0x8e);
    load_idt_entry(46, (unsigned long)irq_15, KERNEL_CODE_SEGMENT, 0x8e);
}

inline void irq_ack(uint32_t irq_number)
{
    if(irq_number >= 8)
    {
        write_port(SLAVE_PIC_CMD, PIC_ACK);
    }
    else
    {
        write_port(MASTER_PIC_CMD, PIC_ACK);
    }
}

//Generic irq handler.
void default_irq_handler(regs_t* registers)
{
    if(irq_handlers[registers->irq_number])
    {
        irq_handlers[registers->irq_number]();
    }
    else
    {
        log_print(LOG_WARNING, "Received IRQ %d but no handler found.", registers->irq_number);
    }

    irq_ack(registers->irq_number);
}

error_code_t register_irq(irq_handler_t irq_handler, uint32_t index)
{
    if(irq_handler == NULL || index < 0 || index > 15)
    {
        return INVALID_ARGUMENT;
    }

    irq_handlers[index] = irq_handler;

    return SUCCESS;
}

error_code_t unregister_irq(irq_handler_t irq_handler, uint32_t index)
{
    if(irq_handler == NULL || index < 0 || index > 15)
    {
        return INVALID_ARGUMENT;
    }

    irq_handlers[index] = NULL;

    return SUCCESS;
}
