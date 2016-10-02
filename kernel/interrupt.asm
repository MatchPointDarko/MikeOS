;MikeOS: Interrupt handlers(Exceptions, IRQS and Software Interrupts)

%macro INTERRUPT_ENTRY 3 

global %1_%2
%1_%2:
    pushad
    push %2
    mov eax, esp
    push eax
    call %3
    add esp, 8
    popad
    iretd

%endmacro

;extern default_exception_handler
extern default_irq_handler
extern default_system_call_handler

section .text
;INTERRUPT_ENTRY 0, default_exception_handler
;INTERRUPT_ENTRY 1, default_exception_handler
;INTERRUPT_ENTRY 2, default_exception_handler
;INTERRUPT_ENTRY 3, default_exception_handler
;INTERRUPT_ENTRY 4, default_exception_handler
;INTERRUPT_ENTRY 5, default_exception_handler
;INTERRUPT_ENTRY 6, default_exception_handler
;INTERRUPT_ENTRY 7, default_exception_handler
;INTERRUPT_ENTRY 8, default_exception_handler
;INTERRUPT_ENTRY 9, default_exception_handler
;INTERRUPT_ENTRY 10, default_exception_handler
;INTERRUPT_ENTRY 11, default_exception_handler
;INTERRUPT_ENTRY 12, default_exception_handler
;INTERRUPT_ENTRY 13, default_exception_handler
;INTERRUPT_ENTRY 14, default_exception_handler
;INTERRUPT_ENTRY 15, default_exception_handler
;INTERRUPT_ENTRY 16, default_exception_handler
;INTERRUPT_ENTRY 17, default_exception_handler
;INTERRUPT_ENTRY 18, default_exception_handler
;INTERRUPT_ENTRY 19, default_exception_handler
;INTERRUPT_ENTRY 20, default_exception_handler
;INTERRUPT_ENTRY 21, default_exception_handler
;INTERRUPT_ENTRY 22, default_exception_handler
;INTERRUPT_ENTRY 23, default_exception_handler
;INTERRUPT_ENTRY 24, default_exception_handler
;INTERRUPT_ENTRY 25, default_exception_handler
;INTERRUPT_ENTRY 26, default_exception_handler
;INTERRUPT_ENTRY 27, default_exception_handler
;INTERRUPT_ENTRY 28, default_exception_handler
;INTERRUPT_ENTRY 29, default_exception_handler
;INTERRUPT_ENTRY 30, default_exception_handler
;INTERRUPT_ENTRY 31, default_exception_handler

; IRQ HANDLERS
INTERRUPT_ENTRY irq, 0, default_irq_handler
INTERRUPT_ENTRY irq, 1, default_irq_handler
INTERRUPT_ENTRY irq, 2, default_irq_handler
INTERRUPT_ENTRY irq, 3, default_irq_handler
INTERRUPT_ENTRY irq, 4, default_irq_handler
INTERRUPT_ENTRY irq, 5, default_irq_handler
INTERRUPT_ENTRY irq, 6, default_irq_handler
INTERRUPT_ENTRY irq, 7, default_irq_handler
INTERRUPT_ENTRY irq, 8, default_irq_handler
INTERRUPT_ENTRY irq, 9, default_irq_handler
INTERRUPT_ENTRY irq, 10, default_irq_handler
INTERRUPT_ENTRY irq, 11, default_irq_handler
INTERRUPT_ENTRY irq, 12, default_irq_handler
INTERRUPT_ENTRY irq, 13, default_irq_handler
INTERRUPT_ENTRY irq, 14, default_irq_handler
INTERRUPT_ENTRY irq, 15, default_irq_handler

; SOFTWARE INTERRUPT HANDLER
INTERRUPT_ENTRY interrupt, 80, default_system_call_handler

