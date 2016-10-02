/*
 * MikeOS: Task implementation. (A task is another name for a thread)
 */
#include <kmalloc.h>
#include <virtual_mm_manager.h>
#include <userspace_manager.h>
#include <common.h>
#include <memory.h>
#include <string.h>
#include <kheap.h>

#define KERNEL_STACK_SIZE (PAGE_SIZE)

typedef struct context
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} context_t;

typedef struct task
{
    virtual_address_space_t* vmm;
    context_t context;
    uint8_t name[20];
    uint32_t kernel_ss;
    uint32_t stack_begin;
    uint32_t stack_end;

    uint32_t text_begin;
    uint32_t text_end;

    uint32_t data_begin;
    uint32_t data_end;

    void* kernel_stack;
} task_t;

task_t* alloc_task(const char* name)
{
    if(name == NULL)
    {
        return NULL;
    }

    task_t* task = kmalloc(sizeof(task_t));

    if(task == NULL)
    {
        return NULL;
    }

    task->vmm = alloc_virtual_address_space();

    if(task->vmm == NULL)
    {
        kfree(task);
        return NULL;
    }

    memcpy(task->name, (char *)name, strlen(name));

    task->kernel_ss = 0x10;
    task->kernel_stack = alloc_kheap_pages(1);

    if(task->kernel_stack == NULL)
    {
        free_virtual_address_space(task->vmm);
        kfree(task);

        return NULL;
    }

    return task;
}

error_code_t free_task(task_t* task)
{
    if(task == NULL)
    {
        return INVALID_ARGUMENT;
    }

    if(free_virtual_address_space(task->vmm) != SUCCESS)
    {
        return FAILURE;
    }

    free_kheap_pages(task->kernel_stack, 1);

    kfree(task);

    return SUCCESS;
}

task_t* load_task(const char* path)
{
    task_t* task = alloc_task(path);

    switch_virtual_address_space(task->vmm);

    set_kernel_stack(((uint32_t)task->kernel_stack) + KERNEL_STACK_SIZE);

    /*
    if(load_elf(task) != SUCCESS)
    {
        return NULL;
    }
    */

    //jmp_to_userspace(task->text_begin, task->stack_end);
}
