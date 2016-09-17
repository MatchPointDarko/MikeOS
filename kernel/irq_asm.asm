;MikeOS: Assembly file of IRQ.

%macro IRQ_ENTRY 1

global irq_%1
irq_%1:
    pushad
    push %1
    mov eax, esp
    push eax
    call default_irq_handler
    add esp, 8
    popad
    iretd

%endmacro

extern default_irq_handler

section .text

IRQ_ENTRY 0
IRQ_ENTRY 1
IRQ_ENTRY 2
IRQ_ENTRY 3
IRQ_ENTRY 4
IRQ_ENTRY 5
IRQ_ENTRY 6
IRQ_ENTRY 7
IRQ_ENTRY 8
IRQ_ENTRY 9
IRQ_ENTRY 10
IRQ_ENTRY 11
IRQ_ENTRY 12
IRQ_ENTRY 13
IRQ_ENTRY 14
IRQ_ENTRY 15
