OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

USER_SPACE_OFFSET = 0x100000;

SECTIONS
{
    . += USER_SPACE_OFFSET;
   .text ALIGN(4K) :  {*(.text) }
   .data ALIGN(4K) :  { *(.data) }
   .rodata ALIGN(4K) : { *(.rodata) }
   .bss  ALIGN(4K) :  { *(.bss)  }
}
