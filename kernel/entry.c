/*
 * MikeOS: Entry point, kmain.
 *
 */

#include "gdt.h"
#include "idt.h"
#include "kmalloc.h"
#include "vga.h"
#include "ata.h"
#include "port_io.h"
#include "stdio.h"
#include "keyboard_driver.h"
#include "multiboot_info.h"
#include "physical_mm_manager.h"
#include "paging_manager.h"
#include "common.h"
#include "logger.h"

/*
 * Booted from harddisk, probably already installed.
 */
static void harddisk_boot()
{




}

/*
 * Booted from a floppy.
 * Create a rootfs, using ramdisk.
 */
static void floppy_boot(multiboot_module_t* initrd)
{
   //Initrd is FAT fs, load it to the root device



}

void kmain(struct multiboot_info* info)
{
    vga_flush();
    log_print(LOG_INFO, "Entered High half kernel");

    log_print(LOG_INFO, "Initializing memory manager");
    phy_memory_manager_init((multiboot_memory_map_t *)info->mmap_addr, info->mmap_length);

    log_print(LOG_INFO, "Remaping the GDT, after virtual memory init");
    remap_gdt();

    log_print(LOG_INFO, "Initializing virtual memory manager");
    virtual_memory_manager_init();

    log_print(LOG_INFO, "Initializing IDT");
    idt_init();

    log_print(LOG_INFO, "Initializing keyboard driver");
    keyboard_init();

    log_print(LOG_INFO, "Initializing ATA driver");
    ata_init();
    HLT();
}
