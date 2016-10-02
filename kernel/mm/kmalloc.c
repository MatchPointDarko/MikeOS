/*
 * MikeOS: Kernel heap allocator, using K&R algorithm
 */
#include <common.h>
#include <mm/kmalloc.h>
#include <mm/paging.h>
#include <mm/kheap.h>

static error_code_t increase_heap_size(uint32_t units);

typedef struct block_header
{
    struct block_header* next_free_block;
    uint32_t block_size;
} block_header_t;

typedef struct block_list_head
{
   block_header_t* free_blocks;

} block_list_head_t;

static block_list_head_t blocks_info = {NULL};

static error_code_t increase_heap_size(uint32_t units)
{
    uint32_t num_pages = (units / PAGE_SIZE) + 1;
    block_header_t* block = alloc_kheap_pages(num_pages);

    if(block == NULL)
    {
       return FAILURE;
    }

    block->block_size = num_pages * PAGE_SIZE;
    block->block_size = ((block->block_size + sizeof(block_header_t) - 1) / sizeof(block_header_t));
    block->block_size--;

    kfree(block + 1);

    return SUCCESS;
}

void kfree(void* address)
{
    block_header_t* block = address;
    block--;

    block_header_t** current = &blocks_info.free_blocks;
    for(; *current != NULL; current = &(*current)->next_free_block);

    *current = block;
    (*current)->next_free_block = NULL;

    //TODO: Merge blocks.
}

void* kmalloc(uint32_t size)
{
    uint32_t units = ((size + sizeof(block_header_t) - 1) / sizeof(block_header_t));
    void* address = NULL;

    for(block_header_t** current = &blocks_info.free_blocks;
        ;
        current = &(*current)->next_free_block)
    {
       if(*current == NULL)
       {
          //Allocate..
          if(increase_heap_size(units) != SUCCESS)
          {
             break;
          }

          //Now current should point to a buffer.
       }

       if((*current)->block_size >= units)
       {
          block_header_t* free_block = NULL;
          if((*current)->block_size > units)
          {
             free_block = ((*current) + (*current)->block_size) - units;
             free_block--;
             free_block->block_size = units;
             free_block->next_free_block = NULL;
             (*current)->block_size -= units;
          }
          else
          {
             free_block = *current;
             *current = (*current)->next_free_block;
          }

          address = free_block + 1;
          break;
       }
    }

    return address;
}
