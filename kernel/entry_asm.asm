bits 32

;grub bootloader header
section .multiboot
align 4
    dd 0x1BADB002 ;magic
    dd 0x00                  ;flags
    dd - (0x1BADB002 + 0x00) ;checksum. m+f+c should be zero

; Declarations
global start
extern kmain
extern paging_init
extern load_gdt

section .init

start:
    cli 			;block interrupts
    mov esp, init_stack
    call load_gdt
    call paging_init

    ;Now high half kernel is mapped to the page directory

    mov esp, stack_space	;set stack pointer
    push ebx ;grub boot info
    call kmain

;4KB small stack for my init section.
resb 4096
init_stack:

;8KB for stack
section .bss
resb 8192
stack_space:
