#include "gdt.h"
#include "idt.h"
#include "kmalloc.h"
#include "terminal.h"
#include "ata.h"
#include "port_io.h"
#include "stdio.h"
#include "keyboard_driver.h"
#include "multiboot_info.h"
#include "physical_mm_manager.h"
#include "paging_manager.h"
#include "logger.h"

void kmain(struct multiboot_info* info)
{
    flush_screen();
    log_print(LOG_INFO, "Entered High half kernel");

    log_print(LOG_INFO, "Initializing memory manager");
    memory_manager_init((multiboot_memory_map_t *)info->mmap_addr, info->mmap_length);

    log_print(LOG_INFO, "Remaping the GDT, after virtual memory init");
    remap_gdt();

    log_print(LOG_INFO, "Initializing virtual memory manager");
    virtual_memory_manager_init();

    log_print(LOG_INFO, "Initializing IDT");
    idt_init();

    log_print(LOG_INFO, "Initializing keyboard driver");
    keyboard_init();

    log_print(LOG_INFO, "Initializing hard-drive driver");

    while(1)
        asm("hlt");
}
