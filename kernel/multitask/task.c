/*
 * MikeOS: Task implementation. (A task is another name for a thread)
 */
#include <mm/kmalloc.h>
#include <mm/virtual_mm_manager.h>
#include <multitask/userspace_manager.h>
#include <multitask/task.h>
#include <mm/kheap.h>
#include <common.h>
#include <libc/memory.h>
#include <libc/string.h>
#include <gdt.h>

#define KERNEL_STACK_SIZE (PAGE_SIZE)
#define STACK_END_ADDRESS (KERNEL_VIRTUAL_OFFSET - PAGE_SIZE)
#define STACK_SIZE (5) // 5 pages

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

    memcpy(task->name, (char *)name, strlen(name) + 1);

    task->kernel_ss = KERNEL_DATA_SEGMENT;
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

    if(task == NULL)
    {
        return NULL;
    }

    switch_virtual_address_space(task->vmm);

    set_kernel_stack(((uint32_t)task->kernel_stack) + KERNEL_STACK_SIZE);

    if(load_elf(task) != SUCCESS)
    {
        free_task(task);
        return NULL;
    }

    if(mmap(STACK_END_ADDRESS - STACK_SIZE * PAGE_SIZE, STACK_SIZE) != SUCCESS)
    {
        free_task(task);
        return NULL;
    }

    task->stack_begin = STACK_END_ADDRESS - STACK_SIZE * PAGE_SIZE;
    task->stack_end = STACK_END_ADDRESS;

    jmp_to_userspace(task->text_begin, task->stack_end);

    return task;
}
