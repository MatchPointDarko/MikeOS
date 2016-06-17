section .text

global keyboard_handler
extern keyboard_handler_main

keyboard_handler:
    pushad
    cld
	call    keyboard_handler_main
    popad
	iretd
