#ifndef MIKE_OS_IRQ_H
#define MIKE_OS_IRQ_H

#include "common.h"
#include "error_codes.h"

typedef void (*irq_handler_t)(void);

void irq_init();
error_code_t register_irq(irq_handler_t irq_handler, uint32_t index);
error_code_t unregister_irq(irq_handler_t irq_handler, uint32_t index);

#endif //MIKE_OS_IRQ
