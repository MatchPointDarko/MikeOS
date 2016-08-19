/*
 * MikeOS: VGA Driver.
 * TODO: VGA module needs refactoring ASAP, ugly, and really really stupid.
 */
#include "vga.h"

#define SCREEN_COLS 80
#define SCREEN_ROWS 25

static void tab_handler();
static void newline_handler();
static void backspace_handler();

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct
{
	char* vga_buffer;
	color_t color;
	unsigned int x, y;
} vga_state_t;

vga_state_t vga_state = {(char*)0xb8000,
						 VGA_DEFAULT_COLOR,
						 0, 0};

static char special_characters[] = {'\n', '\t', '\r', '\b'};
static void(*special_characters_handlers[])() = {newline_handler,
												 tab_handler,
												 backspace_handler};

static unsigned int get_special_character_index(char c)
{
	for(int i = 0; i < ARRAY_SIZE(special_characters); i++)
	{
		if(special_characters[i] == c)
			return i;
	}

	return -1;
}

static inline char* get_address_by_location(unsigned int x, unsigned int y)
{
	return (char*)(vga_state.vga_buffer + ((x * 2) + (y * SCREEN_COLS * 2)));
}

static inline int y_reached_limit()
{
	return vga_state.y > SCREEN_ROWS;
}

static inline int x_reached_limit()
{
	return vga_state.x > SCREEN_ROWS;
}

static void backspace_handler()
{
	if(vga_state.x - 1 >= 0) {
		vga_state.x -= 1;
		vga_putc(' ');
		vga_state.x -= 1;
	}
}

static void tab_handler()
{
	//Tab is 4 spaces.
	vga_state.x += 4;
	if(x_reached_limit())
	{
		newline_handler();
	}
}

static void newline_handler()
{
	vga_state.y++;
	vga_state.x = 0;
	if(y_reached_limit())
	{
		//handle_y_limit();
	}
}

static void default_handler(char c)
{
	char* addr = get_address_by_location(vga_state.x, vga_state.y);
	addr[0] = c;
	addr[1] = vga_state.color;
	vga_state.x++;
}


void vga_flush()
{
	unsigned int i = 0;
	while(i < (SCREEN_COLS * SCREEN_ROWS * 2))
	{
		vga_state.vga_buffer[i] = ' ';
		vga_state.vga_buffer[i + 1] = COLOR_BLACK;
		i += 2;
	}

	vga_state.y = vga_state.x = 0;
}

void vga_putc(const char c)
{
	unsigned int handler_index = 0;

	if((handler_index = get_special_character_index(c)) != -1)
	{
		special_characters_handlers[handler_index]();
	}

	else
	{
		default_handler(c);
	}
}

void vga_print(const char* str)
{
	for(; *str != '\0'; str++)
	{
		vga_putc(*str);
	}
}

void set_terminal_color(color_t color)
{
	if(color >= COLOR_BLUE && color <= COLOR_WHITE)
	{
		vga_state.color = color;
	}
}
