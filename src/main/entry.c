#include "idt.h"
#include "terminal.h"
#include "harddrive.h"
#include "port_io.h"
#include "keyboard_driver.h"
#include "multiboot_info.h"

void kmain(struct multiboot_info* info)
{
    flush_screen();

    idt_init();
    keyboard_init();
    //harddisk_init();
    //memory_manager_init();

    while(1)
    {
        asm("hlt");
    }
}
