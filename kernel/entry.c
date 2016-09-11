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
#include "vfs.h"
#include "memory.h"
#include "panic.h"
#include "list.h"
#include "irq.h"

/*
 * Booted from harddisk, probably already installed.
 */
static void harddisk_boot()
{
}

/*
 * Booted from a floppy.
 * Create a rootfs-> using ramdisk.
 */
static void floppy_boot(multiboot_module_t* initrd)
{
   //Initrd is FAT fs-> load it to the root device
}

static inline void make_initrd_fs(struct multiboot_info* info, file_system_t* fs)
{
    if(info->mods_count != 1)
    {
        kernel_panic();
    }

    memcpy(fs->name, "initrd", sizeof("initrd") + 1);

    fs->read = initrd_read;
    fs->open = initrd_open;
    fs->close = initrd_close;
    fs->partition_begin_offset = ((multiboot_module_t*)info->mods_addr)->mod_start;
    fs->partition_end_offset = ((multiboot_module_t*)info->mods_addr)->mod_end;
    fs->fs_specific = fs->partition_end_offset - fs->partition_begin_offset;
    fs->device = NULL;
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

    log_print(LOG_INFO, "Initializing IRQ Handlers");
    irq_init();

    log_print(LOG_INFO, "Initializing keyboard driver");
    keyboard_init();

    log_print(LOG_INFO, "Initializing ATA driver");
    ata_init();

    log_print(LOG_INFO, "Initializing Virtual file system");
    vfs_init();

    //initrd
    file_system_t fs = {0};

    make_initrd_fs(info, &fs);

    log_print(LOG_INFO, "Mounting initrd at /");
    //Mount the VFS at root dir
    if(vfs_mount("/", &fs) != SUCCESS)
    {
        log_print(LOG_ERROR, "Couldn't mount initrd");
        kernel_panic();
    }

    HLT();
}
