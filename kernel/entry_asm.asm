; MikeOS: Entry point initializaing high-half kernel and paging.

bits 32

;Declarations
global start
extern kmain
extern paging_init
extern load_gdt

section .multiboot
align 4
    dd 0x1BADB002 ;magic
    dd 0x2 ;flags
    dd - (0x1BADB002 + 0x02) ;checksum. m+f+c should be zero

section .init
start:
    cli ;block interrupts
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
