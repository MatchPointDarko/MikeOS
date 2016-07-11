/* Memory allocator, using K&R algorithm
 */
#include "kmalloc.h"
#include "common.h"
#include "paging_manager.h"
#include "physical_mm_manager.h"

#define PAGE_SIZE 4096

typedef long align_t;

typedef struct block_header
{
    struct block_header* next_free_block;
    unsigned long block_size;
    bool is_free;
    align_t alignment;
} block_header_t;

static block_header_t base = {0};
static block_header_t* free_blocks_list = NULL;

static void* increase_heap_size()
{
   block_header_t* insert = NULL;
   void* free_page = allocate_kernel_virtual_page();

   if(free_page == NULL)
   {
      return NULL;
   }

   insert = free_page;
   insert->data.block_size = PAGE_SIZE;

   //Add insert to the end of the linked list.
   kfree(insert + 1);

   return free_blocks_list;
}

void* kmalloc(size_t size)
{
   unsigned long nunits = (size + sizeof(block_header_t) - 1) / (sizeof(block_header_t) + 1);

   //No free list exists, base points to itself, size is 0.
   if(free_blocks_list == NULL)
   {
      free_blocks_list = &base;
      free_blocks_list->next_free_block = NULL;
      free_blocks_list->is_free = false;
      free_blocks_list->block_size = 0;
   }

   //Iterate on this shit.
   void* address = NULL;
   bool searching_for_block = true;

   for(block_header_t* current = free_blocks_list;
       searching_for_block;
       current = current->next_free_block)
   {
      //We haven't found any block that satisfies us,
      //Lets allocate a new page :-(
      if(current->next_free_block == NULL)
      {
         if((current = increase_heap_size()) == NULL)
         {
            searching_for_block = false;
         }
         current->block_size = nunits;
         current->next_free_block =
      }
      else if(current->block_size >= nunits)
      {
         if(current->block_size == nunits)
         {}
         else
         {}
      }
   }

   return address;
   block_header_t* prev = free_blocks_list;

   for(block_header_t* current = prev->data.next_free_block;
        ;
       prev = current, current = current->data.next_free_block)
   {
      if(current->data.block_size >= nunits)
      {
         if(current->data.block_size == nunits)
         {
            //Remove the free block from the free list.
            prev->data.next_free_block = current->data.next_free_block;
         }
         else
         {
            current->data.block_size -= nunits;
            current += current->data.block_size;
            current->data.block_size = nunits;
         }

         free_blocks_list = prev;
         return (void*) (current + 1);
      }

      if(current == free_blocks_list)
      {
         if ((current = increase_heap_size()) == NULL)
         {
            return NULL;
         }
      }
   }
}

/*
 * Add the address to the end of the linked list.
 */
void kfree(void* address)
{
}
