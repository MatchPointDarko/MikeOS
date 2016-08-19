/*
 * MikeOS: IRQ default handler, and irq redirection.
 */

typedef struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int irq_number, err_code;
    unsigned int eip, cs, flags, user_esp, ss;
} regs_t;

//Generic irq handler.
void irq_handler(regs_t* registers)
{


}
