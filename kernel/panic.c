/*
 * MikeOS: Kernel panic.
 */

#include <logging/logger.h>
#include <common.h>

void kernel_panic()
{
    log_print(LOG_ERROR, "KERNEL PANIC!");
    HLT();
}
