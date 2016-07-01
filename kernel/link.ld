OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

KERNEL_VIRTUAL_OFFSET = 0xC0000000;

SECTIONS
{
    . = 1M;
    kernel_start = .;

    start_init = .;
   .init ALIGN(4K) : AT(ADDR(.init) - KERNEL_VIRTUAL_OFFSET)
                          { *(.multiboot);
                            *(.init);
                          }
    end_init = .;
    init_page_table_start = .;

    . += ((end_init - start_init) / 4K) / 1K;

    init_page_table_end = .;

    . += KERNEL_VIRTUAL_OFFSET;

    kernel_high_half_start = .;
    /****** KERNEL *******/
   .text ALIGN(4K) :  AT(ADDR(.text) - KERNEL_VIRTUAL_OFFSET)
                         {*(.text) }
   .data ALIGN(4K) :  AT(ADDR(.data) - KERNEL_VIRTUAL_OFFSET)
                         { *(.data) }
   .bss  ALIGN(4K) :  AT(ADDR(.bss) - KERNEL_VIRTUAL_OFFSET)
                         { *(.bss)  }
    /****** KERNEL *******/
    kernel_high_half_end = .;

    kernel_end = . - KERNEL_VIRTUAL_OFFSET;
}
