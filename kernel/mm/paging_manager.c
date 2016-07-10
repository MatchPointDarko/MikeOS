#include "common.h"
#include "memory.h"
#include "physical_mm_manager.h"
#include "external_linker_symbols.h"

#define PAGE_SIZE 4096
#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define KERNEL_DIRECTORY_ENTRY 768
#define MAX_PAGE_DIRECTORY_INDEX 1023
#define KERNEL_LATER_TABLE_START_ADDRESS (KERNEL_VIRTUAL_OFFSET + (PAGE_SIZE * PAGE_TABLE_SIZE))
#define ADDRESS_TO_DIRECTORY_ENTRY(address) (address >> 22)

/* Page bits */
#define SUPERVISOR(val) val
#define READ_WRITE(val) val | 2
#define PRESENT(val) val | 1
/* Page bits */

typedef int page_table_t;
typedef int page_pointer_t;

page_table_t kernel_page_directory[PAGE_DIR_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables"))) = {0};

//Contains the virtual address, to avoid identity mapping usage
page_table_t* virtual_kernel_page_directory = ((unsigned long)kernel_page_directory) + KERNEL_VIRTUAL_OFFSET;

page_pointer_t kernel_init_page_table[PAGE_TABLE_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables"))) = {0};

/* Identity map the low 1M
 * In early boot stage.
 */
static void __attribute__((section(".init"))) create_page_table()
{
   unsigned int current_page = 0;
   for(int i = 0; i < PAGE_TABLE_SIZE; i++, current_page += PAGE_SIZE)
   {
      kernel_init_page_table[i] = current_page;
      kernel_init_page_table[i] = READ_WRITE(kernel_init_page_table[i]);
      kernel_init_page_table[i] = PRESENT(kernel_init_page_table[i]);
   }
}

/* Map the kernel memory to its page directory,
 * **in early boot stage.
 * We don't need to map the init section, we don't need it anymore.
 */
__attribute__((section(".init"))) static void map_lower_mem()
{
   kernel_page_directory[0] = ((unsigned long)kernel_init_page_table) | 0x3;
}

__attribute__((section(".init"))) static void map_kernel_memory()
{
   kernel_page_directory[KERNEL_DIRECTORY_ENTRY] = ((unsigned long)kernel_init_page_table) | 0x3;
}

__attribute__((section(".init"))) static inline void enable_paging()
{
   //Update page directory register
   asm("lea eax, [kernel_page_directory]");
   asm("mov cr3, eax");
   asm("mov eax, cr0");

   //Turn paging bit on
   asm("or eax, 0x80000000");
   asm("mov cr0, eax");
}

__attribute__((section(".init"))) void paging_init()
{
   create_page_table();
   map_lower_mem();
   map_kernel_memory();
   enable_paging();
}

#define KERNEL_HEAP_SIZE_MB 512
#define PAGE_DIRECTORY_OFFSET 769
#define KERNEL_HEAP_SIZE_B (KERNEL_SIZE_MB * 1024 * 1024)
#define MANAGED_VMEM_START_ADDRESS 0xc0400000
#define PAGE_TABLE_AT(index) page_tables[index]
#define PAGE_AVAILABLE 0xFF
#define PAGE_UNAVAILABLE 0x00
#define NO_AVAILABLE_PAGE -1

static char bitmap[16384];
unsigned long current_free_page_index = 0;
//128 Page tables for 512MB(512MB / 4MB), heap size = 512MB
static page_pointer_t page_tables[128][PAGE_SIZE];


void insert_kernel_page_tables(page_table_t* page_directory)
{
    unsigned long directory_index = MANAGED_VMEM_START_ADDRESS >> 22;
    unsigned long max_index = (MANAGED_VMEM_START_ADDRESS + KERNEL_HEAP_SIZE_B) >> 22;

    //Identity map
    page_directory[0] = kernel_init_page_table;
    page_directory[0] = SUPERVISOR(page_directory[KERNEL_DIRECTORY_ENTRY]);
    page_directory[0] = READ_WRITE(page_directory[KERNEL_DIRECTORY_ENTRY]);
    page_directory[0] = PRESENT(page_directory[KERNEL_DIRECTORY_ENTRY]);

    //Init page table contain the kernel itself
    page_directory[KERNEL_DIRECTORY_ENTRY] = kernel_init_page_table;
    page_directory[KERNEL_DIRECTORY_ENTRY] = SUPERVISOR(page_directory[KERNEL_DIRECTORY_ENTRY]);
    page_directory[KERNEL_DIRECTORY_ENTRY] = READ_WRITE(page_directory[KERNEL_DIRECTORY_ENTRY]);
    page_directory[KERNEL_DIRECTORY_ENTRY] = PRESENT(page_directory[KERNEL_DIRECTORY_ENTRY]);

    //Heap page tables
    for(directory_index; directory_index <= max_index; directory_index++)
    {
        page_directory[directory_index] = &page_tables[directory_index - PAGE_DIRECTORY_OFFSET][0];
        page_directory[directory_index] = SUPERVISOR(page_directory[directory_index]);
        page_directory[directory_index] = READ_WRITE(page_directory[directory_index]);
        page_directory[directory_index] = PRESENT(page_directory[directory_index]);
    }
}

void virtual_memory_manager_init()
{
    //Map all of the page tables to kernel page directory
    insert_kernel_page_tables(kernel_page_directory);

    //All pages available
    memset(PAGE_AVAILABLE, bitmap, sizeof(bitmap));
}

void* allocate_kernel_virtual_page()
{
    if(current_free_page_index == NO_AVAILABLE_PAGE)
    {
        // :-(
        return NULL;
    }

    unsigned char bit_index = current_free_page_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(current_free_page_index);

    turn_bit_off(&bitmap[bitmap_index], bit_index);

    void* free_page_addr = (void*)(MANAGED_VMEM_START_ADDRESS + (PAGE_SIZE * (current_free_page_index)));

    set_next_free_page_index();

    return free_page_addr;
}

void free_kernel_virtual_page(void* virtual_address)
{



}

/* Map a physical page to kernel virtual addresses
 * ****Assuming it is aligned to page boundary!!!!****
 */
void* kernel_physical_to_virtual(void* phys_address)
{
   static page_table_t* current_page_table = NULL;
   static unsigned int current_page_table_dir_index = ADDRESS_TO_DIRECTORY_ENTRY(KERNEL_LATER_TABLE_START_ADDRESS);
   static unsigned int current_page_table_index = 0;

   if(current_page_table_dir_index > MAX_PAGE_DIRECTORY_INDEX)
   {
       //We're trying to allocate more than 1GB
       //SHOULD NEVER HAPPEN, but just in case..
       return NULL;
   }

   if(current_page_table == NULL)
   {
      current_page_table = allocate_physical_page();
      virtual_kernel_page_directory[current_page_table_dir_index] = current_page_table;
   }

   if(current_page_table_index == PAGE_TABLE_SIZE)
   {
      //Allocate a new page table
      current_page_table = allocate_physical_page();
      current_page_table_dir_index++;
      current_page_table_index = 0;
      virtual_kernel_page_directory[current_page_table_dir_index] = current_page_table;
   }

   current_page_table[current_page_table_index] = phys_address;
   current_page_table[current_page_table_index] = SUPERVISOR(current_page_table[current_page_table_index]);
   current_page_table[current_page_table_index] = READ_WRITE(current_page_table[current_page_table_index]);
   current_page_table[current_page_table_index] = PRESENT(current_page_table[current_page_table_index]);

   unsigned int page_table_base_address = (current_page_table_dir_index * (PAGE_TABLE_SIZE * PAGE_SIZE));

   return page_table_base_address + (current_page_table_index++ * PAGE_SIZE);
}
