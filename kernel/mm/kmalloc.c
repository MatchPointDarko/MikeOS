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
   insert->block_size = (PAGE_SIZE / sizeof(block_header_t)) - 1;
   insert->next_free_block = NULL;

   if(free_blocks_list == &base)
   {
      free_blocks_list = insert;
   }
   else
   {
      kfree(insert + 1);
   }

   return free_page;
}

void* kmalloc(size_t size)
{
   unsigned long nunits = ((size + sizeof(block_header_t) - 1) / sizeof(block_header_t));

   if(size > PAGE_SIZE - sizeof(block_header_t))
   {
      //Currently doesn't support size that extends page size.
      return NULL;
   }
   //No free list exists, base points to itself, size is 0.
   if(free_blocks_list == NULL)
   {
      free_blocks_list = &base;
      free_blocks_list->next_free_block = NULL;
      free_blocks_list->block_size = 0;
   }
   void* address = NULL;
   block_header_t* prev = NULL;
   bool_t searching_for_block = true;

   for(block_header_t* current = free_blocks_list;
       searching_for_block;
       prev = current, current = current->next_free_block)
   {
      if(current->block_size >= nunits)
      {
         if(current->block_size == nunits)
         {
            //Bullseye
            if(current == NULL)
            {
               //Head of the list
               free_blocks_list = current->next_free_block;
            }
            else
            {
               prev->next_free_block = current->next_free_block;
            }
            address = (void*)(current + 1);
         }
         else
         {
            current->block_size -= nunits;
            current += current->block_size;
            current->block_size = nunits;
            address = (void*)(current + 1);
         }

         searching_for_block = false;
      }

      else if(current->next_free_block == NULL)
      {
         block_header_t* tmp = NULL;
         if((tmp = increase_heap_size()) != NULL)
         {
            current->next_free_block = tmp;
         }
         else
         {
             searching_for_block = false;
         }
      }
   }

   return address;
}

/*
 * Add the address to the end of the linked list.
 * NOTE: Assumes the address is valid! i have no way of checking,
 * i can add a magic number, but comon -,-
 */
void kfree(void* address)
{
   block_header_t* free_block = address;
   free_block--;

   //Iterate until the last free block is found
   block_header_t* it = free_blocks_list;

   for(; it->next_free_block != NULL; it = it->next_free_block)
      ;

   it->next_free_block = free_block;
}
