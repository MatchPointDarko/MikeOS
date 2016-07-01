bits 32
section .multiboot
;grub bootloader header
        align 4
        dd 0x1BADB002            ;magic
        dd 0x00                  ;flags
        dd - (0x1BADB002 + 0x00) ;checksum. m+f+c should be zero

global start
extern kmain	        
extern paging_init

section .init

start:
  cli 			;block interrupts
  mov esp, init_stack
  call paging_init

  ; Now high half kernel is mapped to the page directory
  mov esp, stack_space	;set stack pointer
  push ebx ; grub boot info
  call kmain

loop:
  hlt		 	;halt the CPU
  jmp loop

resb 4096; 4KB small stack for my init section.
init_stack:

section .bss
resb 8192 ;8KB for stack
stack_space:
