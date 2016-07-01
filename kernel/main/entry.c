#include "gdt.h"
#include "idt.h"
#include "terminal.h"
#include "harddrive.h"
#include "port_io.h"
#include "stdio.h"
#include "keyboard_driver.h"
#include "multiboot_info.h"
#include "mm_manager.h"

void kmain(struct multiboot_info* info)
{
    flush_screen();
    load_gdt();

    idt_init();
    keyboard_init();
    memory_manager_init((multiboot_memory_map_t *)info->mmap_addr, info->mmap_length);

    while(1)
        asm("hlt");
}
