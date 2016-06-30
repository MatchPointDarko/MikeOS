OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

//Load kernel to physical 1M, but different virtual
KERNEL_VIRTUAL_OFFSET = 0xC0000000;

SECTIONS
{
    . = 1M;
    kernel_start = .;
   .multiboot ALIGN(4K) : AT(ADDR(.multiboot) - KERNEL_VIRTUAL_OFFSET)
                          { *(.multiboot) }
    . = 1M + KERNEL_VIRTUAL_OFFSET;
   .text ALIGN(4K) :  AT(ADDR(.text) - KERNEL_VIRTUAL_OFFSET)
                         {*(.text) }
   .data ALIGN(4K) :  AT(ADDR(.data) - KERNEL_VIRTUAL_OFFSET)
                         { *(.data) }
   .bss  ALIGN(4K) :  AT(ADDR(.bss) - KERNEL_VIRTUAL_OFFSET)
                         { *(.bss)  }

    kernel_page_directory = .;
    . += 1024 * 24;

    kernel_end = .;
}
