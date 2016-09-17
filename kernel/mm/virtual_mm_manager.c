#if 0
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
#endif