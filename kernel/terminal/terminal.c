/*
 *	Terminal module
 *  System call for writing strings to the VGA terminal,
 */
#include "terminal.h"

#define SCREEN_COLS 80
#define SCREEN_ROWS 25

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

enum vga_color { COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

static int cursor_x = 0;
static int cursor_y = 0;
static char *vga_ptr = (char*)0xb8000;
static enum vga_color terminal_color = COLOR_LIGHT_GREY;

static void tab_handler();
static void new_line_handler();
static void backspace_handler();

static char ESCAPE_CHARACTERS[] = {'\n', '\t', '\b'};
static void (*(ESCAPE_CHARACTERS_HANDLERS[]))() =
		{new_line_handler, tab_handler, backspace_handler};

static inline int is_escape_character(char c)
{
	for(int i = 0; i < ARRAY_SIZE(ESCAPE_CHARACTERS); i++)
	{
		if(ESCAPE_CHARACTERS[i] == c)
			return i;
	}

	return -1;
}

static inline char* get_address_by_location(unsigned int x, unsigned int y)
{
	return (char*)(vga_ptr + ((x * 2) + (y * SCREEN_COLS * 2)));
}

static inline int y_reached_limit()
{
	return cursor_y > SCREEN_ROWS;
}

static inline int x_reached_limit()
{
	return cursor_x > SCREEN_ROWS;
}

static void backspace_handler()
{
	if(cursor_x - 1 >= 0) {
		cursor_x -= 1;
		putc(' ');
		cursor_x -= 1;
	}
}

static void tab_handler()
{
	//Tab is 4 spaces.
	cursor_x += 4;
	if(x_reached_limit())
	{
		new_line_handler();
	}
}

static void new_line_handler()
{
	cursor_y++;
	cursor_x = 0;
	if(y_reached_limit())
	{
		//handle_y_limit();
	}
}

static void default_handler(char c)
{
	char* addr = get_address_by_location(cursor_x, cursor_y);
	addr[0] = c;
	addr[1] = terminal_color;
	cursor_x++;
}

void putc(char c)
{
	int handler_index = 0;

	if((handler_index = is_escape_character(c)) != -1)
	{
		ESCAPE_CHARACTERS_HANDLERS[handler_index]();
		return;
	}
	default_handler(c);
}

void flush_screen()
{
	unsigned int i = 0;
	while(i < (SCREEN_COLS * SCREEN_ROWS * 2))
	{
		vga_ptr[i] = ' ';
		vga_ptr[i + 1] = COLOR_BLACK;
		i += 2;
	}
}

void terminal_print(char* str)
{
	for(; *str != '\0'; str++)
	{
		putc(*str);
	}
}