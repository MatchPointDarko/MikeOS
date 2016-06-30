#include "terminal.h"
#include "port_io.h"
#include "idt.h"
#include "keyboard_driver.h"

#define KERNEL_SEGMENT_CODE 0x08
#define KEYBOARD_IDT_INDEX 0x21

static const unsigned char keyboard_map[128] =
{
   0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
   0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
  '\'', '`',   0,		/* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
   0,	/* Alt */
   ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
    '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
    '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static int current_loc = 0;
static char *vidptr = (char*)0xb8000; 	//video mem begins here.

void keyboard_init(void)
{
    load_idt_entry(KEYBOARD_IDT_INDEX, (unsigned long) keyboard_handler, KERNEL_SEGMENT_CODE, 0x8e);
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void keyboard_irq(void)
{
    char keycode;
    unsigned char status;
    char character_string[2] = {0, 0};

    status = read_port(0x64);
    /* Lowest bit of status will be set if buffer is not empty */
    if (status & 0x01)
    {
        keycode = read_port(0x60);
        if(keycode >= 0 && keyboard_map[keycode])
        {
            character_string[0] = keyboard_map[keycode];
            terminal_print(character_string);
        }
    }

    write_port(0x20, 0x20);
}
