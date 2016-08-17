#include "common.h"
#include "memory.h"
#include "bitmap_manipulation.h"
#include "physical_mm_manager.h"
#include "common_constants.h"

#define PAGE_SIZE 4096
#define PAGE_DIR_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define KERNEL_DIRECTORY_ENTRY 768
#define MAX_PAGE_DIRECTORY_INDEX 1023
#define KERNEL_LATER_TABLE_START_ADDRESS (KERNEL_VIRTUAL_OFFSET + (PAGE_SIZE * PAGE_TABLE_SIZE))
#define ADDRESS_TO_DIRECTORY_ENTRY(address) (address >> 22)

/* Page bits */
#define SUPERVISOR(val) (val)
#define READ_WRITE(val) (void*)((unsigned long)val | 2)
#define PRESENT(val) (void*)((unsigned long)val | 1)
/* Page bits */

typedef unsigned int page_table_t;
typedef unsigned int page_pointer_t;

page_table_t* kernel_page_directory[PAGE_DIR_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables"))) = {0};

//Contains the virtual address, to avoid identity mapping usage
page_table_t* virtual_kernel_page_directory = (page_table_t*)
                                               ((unsigned long)kernel_page_directory) + KERNEL_VIRTUAL_OFFSET;

page_pointer_t* kernel_init_page_table[PAGE_TABLE_SIZE]
__attribute__((aligned(PAGE_SIZE))) __attribute__((section(".tables"))) = {0};

/* Identity map the low 1M
 * In early boot stage.
 */
static void __attribute__((section(".init"))) create_page_table()
{
   unsigned int current_page = 0;
   for(int i = 0; i < PAGE_TABLE_SIZE; i++, current_page += PAGE_SIZE)
   {
      kernel_init_page_table[i] = (page_pointer_t*)current_page;
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
   kernel_page_directory[0] = (page_table_t *)(((unsigned long)kernel_init_page_table) | 0x3);
}

__attribute__((section(".init"))) static void map_kernel_memory()
{
   kernel_page_directory[KERNEL_DIRECTORY_ENTRY] = (page_table_t *)(((unsigned long)kernel_init_page_table) | 0x3);
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
#define KERNEL_HEAP_SIZE_B (KERNEL_HEAP_SIZE_MB * 1024 * 1024)
#define MANAGED_VMEM_START_ADDRESS 0xc0400000
#define PAGE_TABLE_AT(index) page_tables[index]
#define PAGE_AVAILABLE 0xFF
#define PAGE_UNAVAILABLE 0x00

static char bitmap[16384];
static unsigned long current_free_page_index = 0;
//128 Page tables for 512MB(512MB / 4MB), heap size = 512MB
static page_pointer_t kernel_page_tables[128][PAGE_TABLE_SIZE]
__attribute__((section(".tables"))) __attribute__((aligned(PAGE_SIZE))) = {0};

static inline void tlb_flush()
{
    asm("invlpg [0]");
}

void insert_kernel_page_tables(page_table_t** page_directory)
{
    unsigned long directory_index = MANAGED_VMEM_START_ADDRESS >> 22;
    unsigned long max_index = (MANAGED_VMEM_START_ADDRESS + KERNEL_HEAP_SIZE_B) >> 22;

    //Identity map
    page_directory[0] = (page_table_t*)((unsigned long)kernel_init_page_table | 0x3);

    //Init page table contain the kernel itself
    page_directory[KERNEL_DIRECTORY_ENTRY] = (page_table_t*)((unsigned long)kernel_init_page_table | 0x3);

    //Heap page tables
    for(directory_index; directory_index < max_index; directory_index++)
    {
        page_table_t* current_page_table = &kernel_page_tables[directory_index - PAGE_DIRECTORY_OFFSET][0];
        page_directory[directory_index] = current_page_table;
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
        return NULL;
    }

    unsigned char bit_index = current_free_page_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(current_free_page_index);

    turn_bit_off(&bitmap[bitmap_index], bit_index);

    void* free_page_addr = (void*)(MANAGED_VMEM_START_ADDRESS + (PAGE_SIZE * (current_free_page_index)));

    set_next_free_page_index(bitmap, sizeof(kernel_page_tables), &current_free_page_index);

    //Map virtual to physical
    unsigned long free_page_addr_dir_entry = (unsigned long)free_page_addr >> 22;
    unsigned long free_page_addr_table_index = ((unsigned long)free_page_addr >> 12) & 0x3ff;

    //Get a physical page
    void* phys_page = allocate_physical_page();

    //Make it valid and present
    phys_page = SUPERVISOR(phys_page);
    phys_page = READ_WRITE(phys_page);
    phys_page = PRESENT(phys_page);

    //Remove the OR'd value:
    kernel_page_directory[free_page_addr_dir_entry] = (page_table_t*)
                                                      ((unsigned long)kernel_page_directory[free_page_addr_dir_entry]
                                                       & 0xFFFFFFF0);

    kernel_page_directory[free_page_addr_dir_entry][free_page_addr_table_index] = (unsigned long)phys_page;

    //Set bits
    kernel_page_directory[free_page_addr_dir_entry] = SUPERVISOR(kernel_page_directory[free_page_addr_dir_entry]);
    kernel_page_directory[free_page_addr_dir_entry] = READ_WRITE(kernel_page_directory[free_page_addr_dir_entry]);
    kernel_page_directory[free_page_addr_dir_entry] = PRESENT(kernel_page_directory[free_page_addr_dir_entry]);

    tlb_flush();

    return free_page_addr;
}

bool_t free_kernel_virtual_page(void* virtual_address)
{
    if((unsigned long)virtual_address < MANAGED_VMEM_START_ADDRESS)
    {
        return false;
    }

    //TODO:Free the physical page

    //Free the virtual page.
    unsigned long bit_index = address_to_bit_index(virtual_address,
                                                   (void*)MANAGED_VMEM_START_ADDRESS);

    unsigned long bit_location = bit_index % (sizeof(char) * 8);
    unsigned long bitmap_index = bit_index_to_byte_index(bit_index);

    turn_bit_on(&bitmap[bitmap_index], bit_location);

    //Check bit index is lower than the current free page
    if(bit_index < current_free_page_index)
    {
        current_free_page_index = bit_index;
    }

    unsigned long aligned_addr = MANAGED_MEMORY_PAGE_ALIGN(virtual_address, MANAGED_VMEM_START_ADDRESS);
    unsigned long addr_dir_entry = aligned_addr >> 22;
    unsigned long addr_table_index = (aligned_addr >> 12) & 0x3ff;

    kernel_page_directory[addr_dir_entry][addr_table_index] = 0;
    kernel_page_directory[addr_dir_entry] = (page_table_t *)
                                            (((unsigned long)kernel_page_directory[addr_dir_entry]) & 0xFFFFFFF0);

    tlb_flush();

    return true;
}
