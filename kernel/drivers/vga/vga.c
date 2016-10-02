/*
 * MikeOS: VGA Driver.
 * TODO: VGA module needs refactoring ASAP, ugly, and really really stupid.
 */
#include <vga.h>
#include <common.h>
#include <kheap.h>

#define SCREEN_COLS 80
#define SCREEN_ROWS 25
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define VGA_WORD_TO_CHAR(word) ((unsigned char)(word >> 8))
#define VGA_PHYSICAL_ADDRESS (0xb8000)

static void tab_handler();
static void newline_handler();
static void backspace_handler();
static void vga_scroll();

typedef struct
{
	char* vga_buffer;
	color_t color;
	uint32_t x, y;
} vga_state_t;

vga_state_t vga_state = {(char*)VGA_PHYSICAL_ADDRESS,
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
	return vga_state.vga_buffer + ((x * 2) + (y * SCREEN_COLS * 2));
}

static inline int y_reached_limit()
{
	return vga_state.y > SCREEN_ROWS;
}

static inline int x_reached_limit()
{
	return vga_state.x > SCREEN_COLS;
}

static void backspace_handler()
{
	if(vga_state.x - 1 >= 0)
    {
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
        //vga_scroll();
        vga_flush();
	}
}

static void default_handler(char c)
{
	char* addr = get_address_by_location(vga_state.x, vga_state.y);
	addr[0] = c;
	addr[1] = vga_state.color;

	vga_state.x++;

    if(x_reached_limit())
    {
       newline_handler();
    }
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

static void vga_scroll()
{
    //Find the second line
    uint16_t* rows_iter = (uint16_t*)vga_state.vga_buffer;
    while(VGA_WORD_TO_CHAR(*rows_iter++) != '\n');

    //(SCREEN_ROWS - 1) lines
    uint32_t rows_passed = 0;
    uint16_t* buffer_iter = (uint16_t*)vga_state.vga_buffer;
    while(rows_passed != (SCREEN_ROWS - 1))
    {
       *buffer_iter = *rows_iter;
       if(VGA_WORD_TO_CHAR(*buffer_iter) == '\n')
       {
           rows_passed++;
       }

        buffer_iter++;
        rows_iter++;
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

void vga_init()
{
	uint32_t number_of_pages = 2;
	char* vga_buffer = map_physical_to_kheap(VGA_PHYSICAL_ADDRESS, 2);
	vga_state.vga_buffer = vga_buffer;
}
