//
// Created by sourcer on 01/07/16.
//

#ifndef MIKE_OS_EXTERNAL_LINKER_SYMBOLS_H
#define MIKE_OS_EXTERNAL_LINKER_SYMBOLS_H

extern unsigned long start_init, end_init;
extern unsigned long kernel_virtual_start_addr;
extern unsigned long init_page_table_start, init_page_table_end;
extern unsigned long kernel_start;
extern unsigned long kernel_end;
extern unsigned long kernel_high_half_start;
extern unsigned long kernel_high_half_end;

#define KERNEL_START (unsigned long)&kernel_start
#define KERNEL_END (unsigned long)&kernel_end
#define KERNEL_SIZE (KERNEL_END) - (KERNEL_START)
#define KERNEL_VIRTUAL_OFFSET 0xC0000000

#define INIT_START (unsigned long)&start_init
#define INIT_END (unsigned long)&end_init
#define INIT_SIZE (INIT_END) - (INIT_START)
#define KERNEL_VIRTUAL_START (unsigned long)&kernel_high_half_start
#define KERNEL_VIRTUAL_END (unsigned long)&kernel_high_half_end

#endif //MIKE_OS_EXTERNAL_LINKER_SYMBOLS_H
