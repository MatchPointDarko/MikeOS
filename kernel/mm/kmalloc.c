/* Memory allocator, using K&R algorithm
 */
#include "kmalloc.h"
#include "paging_manager.h"
#include "physical_mm_manager.h"

#define PAGE_SIZE 4096

typedef long align_t;

typedef struct block_header
{
   union {
        struct block_header* next_free_block;
        unsigned long block_size;
   } data;

   align_t alignment;
} block_header_t;

static block_header_t base = {0};
static block_header_t* free_blocks = NULL;

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

   kfree(insert + 1);

   return free_blocks;
}

void* kmalloc(size_t size)
{
   unsigned long nunits = (size + sizeof(block_header_t) - 1) / (sizeof(block_header_t) + 1);

   //No free list exists, base points to itself, size is 0.
   if(free_blocks == NULL)
   {
      free_blocks = &base;
      free_blocks->data.next_free_block = &base;

      free_blocks->data.block_size = 0;
   }

   block_header_t* prev = free_blocks;

   printf("got here\n");
   return NULL;
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

         printf("returning : %x\n", current);
         free_blocks = prev;
         return (void*) (current + 1);
      }

      if(current == free_blocks)
      {
         printf("here i am \n");
         if ((current = increase_heap_size()) == NULL)
         {
            return NULL;
         }
      }
   }
}

void kfree(void* address)
{
   block_header_t *insert = NULL, *current = NULL;

   insert = ((block_header_t *) address) - 1;

   for (current = free_blocks;
        !((current < insert) && (insert < current->data.next_free_block));
        current = current->data.next_free_block) {
      if ((current >= current->data.next_free_block)
          && ((current < insert) || (insert < current->data.next_free_block))) {
         break;
      }
   }

   if (insert + insert->data.block_size == current->data.next_free_block)
   {
      insert->data.next_free_block += current->data.next_free_block->data.block_size;
      insert->data.next_free_block = current->data.next_free_block->data.next_free_block;
   }
   else
   {
      insert->data.next_free_block = current->data.next_free_block;
   }

   if ((current + current->data.block_size) == insert)
   {
      current->data.block_size += insert->data.block_size;
      current->data.next_free_block = insert->data.next_free_block;
   }
   else
   {
      current->data.next_free_block = insert;
   }

   free_blocks = current;
}
