#include "memory.h"

#define PAGE_SIZE 4096
#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define BIOS_PAGE_TABLE_ENTRIES 256
#define KERNEL_VIRTUAL_OFFSET 0xC0000000
#define BIOS_DIRECTORY_ENTRY 0
#define KERNEL_DIRECTORY_ENTRY 768

/* Page bits */
#define SUPERVISOR 0
#define READ_WRITE 2
#define PAGE_PRESENT 1
/* Page bits */

typedef int page_table_t;
typedef int page_pointer_t;

extern page_table_t kernel_page_directory[PAGE_DIR_SIZE];

page_pointer_t bios_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PAGE_SIZE)));

extern unsigned long kernel_start;
extern unsigned long kernel_end;

/* Identity map the low 1M */
static void map_bios()
{
   memset(0, bios_page_table, PAGE_TABLE_SIZE);

   unsigned int current_page = 0;
   for(int i = 0; i < BIOS_PAGE_TABLE_ENTRIES; i++, current_page += PAGE_SIZE)
      bios_page_table[i] = (current_page) | (SUPERVISOR)
                                          | (PAGE_PRESENT)
                                          | (READ_WRITE);

   kernel_page_directory[0] = (bios_page_table) | (SUPERVISOR)
                                                | (PAGE_PRESENT)
                                                | (READ_WRITE);
}

/* Map the kernel memory to its page directory. */
static void map_kernel_memory()
{}

static inline void enable_paging()
{
   //TODO: Find a better solution for the magic number...
   asm("mov eax, kernel_page_directory - 0xC0000000");

   asm("mov cr3, eax");
   asm("mov eax, cr0");

   //Turn on paging bit
   asm("or eax, 0x80000000");
   asm("mov cr0, eax");
}

void paging_init()
{
   map_bios();

   map_kernel_memory();

   enable_paging();
}
