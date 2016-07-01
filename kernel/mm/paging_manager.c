#include "memory.h"
#include "external_linker_symbols.h"

#define PAGE_SIZE 4096
#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define BIOS_PAGE_TABLE_ENTRIES 256
#define BIOS_DIRECTORY_ENTRY 0
#define KERNEL_DIRECTORY_ENTRY 768

/* Page bits */
#define SUPERVISOR 0
#define READ_WRITE 2
#define PAGE_PRESENT 1
/* Page bits */

typedef int page_table_t;
typedef int page_pointer_t;

page_table_t kernel_page_directory[PAGE_DIR_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables")));

page_pointer_t kernel_page_tables[PAGE_TABLE_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables")));

page_pointer_t identity_page_table[PAGE_TABLE_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables")));

/* Identity map the low 1M
 * In early boot stage.
 */
static void __attribute__((section(".init"))) map_identity()
{
   //memset(0, identity_page_table, PAGE_TABLE_SIZE);

   unsigned int current_page = 0;
   for(int i = 0; i < BIOS_PAGE_TABLE_ENTRIES; i++, current_page += PAGE_SIZE)
      identity_page_table[i] = (current_page) | (SUPERVISOR)
                                          | (PAGE_PRESENT)
                                          | (READ_WRITE);

   for(int i = 0; i < (INIT_SIZE / PAGE_SIZE); i++, current_page += PAGE_SIZE)
   {
       identity_page_table[i] = (current_page) | (SUPERVISOR)
                                          | (PAGE_PRESENT)
                                          | (READ_WRITE);
   }

   kernel_page_directory[0] = ((unsigned long)(identity_page_table)) | (SUPERVISOR)
                                                | (PAGE_PRESENT)
                                                | (READ_WRITE);
}

/* Map the kernel memory to its page directory,
 * **in early boot stage.
 * We don't need to map the init section, we don't need it anymore.
 */
__attribute__((section(".init"))) static void map_kernel_memory()
{
   //Identity map the init section
   //Start at 1MB i.e. its page aligned.
   unsigned int start_index = INIT_SIZE / PAGE_SIZE;
   unsigned long current_page = KERNEL_VIRTUAL_START - KERNEL_VIRTUAL_OFFSET;

   for(page_pointer_t* page_table_entry = kernel_page_tables + start_index;
       page_table_entry < page_table_entry + (PAGE_TABLE_SIZE - start_index)
       && current_page < KERNEL_END;
       page_table_entry++, current_page += PAGE_SIZE)
   {
      *page_table_entry = (current_page) | (SUPERVISOR)
                                          | (PAGE_PRESENT)
                                          | (READ_WRITE);
   }

   kernel_page_directory[KERNEL_DIRECTORY_ENTRY] = kernel_page_tables;
}

__attribute__((section(".init"))) static inline void enable_paging()
{
   asm("mov eax, kernel_page_directory");

   asm("mov cr3, eax");
   asm("mov eax, cr0");

   //Turn on paging bit
   asm("or eax, 0x80000000");
   asm("mov cr0, eax");
}

__attribute__((section(".init"))) void paging_init()
{
   map_identity();

   //map_kernel_memory();

   enable_paging();
}
