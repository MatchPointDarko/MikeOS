section .text

global keyboard_handler
extern keyboard_irq

keyboard_handler:
    pushad
    cld
	call    keyboard_irq
    popad
	iretd
