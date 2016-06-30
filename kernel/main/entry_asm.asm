bits 32
section .multiboot
;grub bootloader header
        align 4
        dd 0x1BADB002            ;magic
        dd 0x00                  ;flags
        dd - (0x1BADB002 + 0x00) ;checksum. m+f+c should be zero

global start
extern kmain	        

section .text

start:
  cli 			;block interrupts
  mov esp, stack_space	;set stack pointer
  push ebx ; grub boot info
  call kmain

loop:
  hlt		 	;halt the CPU
  jmp loop

section .bss
resb 8192 ;8KB for stack
stack_space:
