OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

KERNEL_VIRTUAL_OFFSET = 0xC0000000;

SECTIONS
{
    . = 1M;
    kernel_start = .;

    start_init = .;
   .init ALIGN(4K) :
                          { *(.multiboot);
                            *(.init);
                            *(.tables);
                          }
    end_init = .;

    . += KERNEL_VIRTUAL_OFFSET;

    kernel_high_half_start = .;
   .text ALIGN(4K) :  AT(ADDR(.text) - KERNEL_VIRTUAL_OFFSET)
                         {*(.text) }
   .data ALIGN(4K) :  AT(ADDR(.data) - KERNEL_VIRTUAL_OFFSET)
                         { *(.data) }
   .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_OFFSET)
                         { *(.rodata) }
   .bss  ALIGN(4K) :  AT(ADDR(.bss) - KERNEL_VIRTUAL_OFFSET)
                         { *(.bss)  }
    kernel_high_half_end = .;

    kernel_end = . - KERNEL_VIRTUAL_OFFSET;
}
