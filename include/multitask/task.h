#ifndef MIKE_OS_TASK_H
#define MIKE_OS_TASK_H

#include <common.h>
#include <error_codes.h>
#include <mm/virtual_mm_manager.h>

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

    uint32_t stack_begin;
    uint32_t stack_end;

    uint32_t text_begin;
    uint32_t text_end;

    uint32_t data_begin;
    uint32_t data_end;

    uint32_t flags;

    uint32_t kernel_ss;
    void* kernel_stack;
} task_t;

task_t* load_task(const char* path);
task_t* alloc_task(const char* name);
error_code_t free_task(task_t* task);

#endif //MIKE_OS_TASK_H
