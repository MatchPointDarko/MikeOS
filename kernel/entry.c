/*
 * MikeOS: Entry point, kmain.
 *
 */

#include <gdt.h>
#include <idt.h>
#include <mm/kmalloc.h>
#include <mm/physical_mm_manager.h>
#include <mm/virtual_mm_manager.h>
#include <multitask/userspace_manager.h>
#include <drivers/vga/vga.h>
#include <drivers/ata/ata.h>
#include <port_io/port_io.h>
#include <libc/stdio.h>
#include <drivers/keyboard/keyboard_driver.h>
#include <multiboot_info.h>
#include <mm/paging.h>
#include <common.h>
#include <logging/logger.h>
#include <fs/vfs.h>
#include <libc/memory.h>
#include <panic.h>
#include <data_structures/list.h>
#include <irq.h>
#include <mm/kheap.h>
#include <fs/file.h>
#include <syscall.h>

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

    if(register_ramdisk(fs->partition_begin_offset, fs->partition_end_offset)
                != SUCCESS)
    {
        log_print(LOG_ERROR, "Couldn't register the ramdisk"
                             "to the physical memory manager");
        kernel_panic();
    }

    void* virtual_address = NULL;
    uint32_t num_pages = (fs->partition_end_offset - fs->partition_begin_offset) / PAGE_SIZE;
    virtual_address = map_physical_to_kheap(fs->partition_begin_offset, num_pages);

    if(!virtual_address)
    {
        log_print(LOG_ERROR, "Couldn't register the ramdisk to the kheap");
        kernel_panic();
    }

    fs->partition_begin_offset = (uint32_t)virtual_address;
    fs->partition_end_offset = fs->partition_begin_offset + num_pages * PAGE_SIZE;
}

void kmain(struct multiboot_info* info)
{
    vga_flush();
    log_print(LOG_INFO, "Entered High half kernel");

    log_print(LOG_INFO, "Initializing physical memory manager");
    phy_memory_manager_init((multiboot_memory_map_t *)info->mmap_addr, info->mmap_length);

    log_print(LOG_INFO, "Remaping the GDT, after virtual memory init");
    remap_gdt();

    log_print(LOG_INFO, "Initializing kernel heap");
    kheap_init();

    log_print(LOG_INFO, "Initializing VGA to its virtual memory form");
    vga_init();

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

    log_print(LOG_INFO, "Initializing system-calls");
    system_calls_init();

    log_print(LOG_INFO, "Initializing userspace");
    userspace_init();
    void load_task(const char* path);

    load_task("sh");

    HLT();
}
