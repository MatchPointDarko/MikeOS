#include "vga.h"
#include "stdio.h"
#include "logger.h"

#define COLOR_INFO COLOR_LIGHT_BLUE
#define COLOR_WARNING COLOR_LIGHT_MAGENTA
#define COLOR_DEBUG COLOR_BLUE
#define COLOR_ERROR COLOR_RED

extern int print(char **out, const char *format, va_list args);

char* LOG_PREFIXS[] = {"[INFO]:", "[WARNING]:", "[ERROR]:", "[DEBUG]:"};
color_t colors[] = {COLOR_INFO, COLOR_WARNING, COLOR_ERROR, COLOR_DEBUG};

void log_print(log_type_t log_type, char* format, ...)
{
    va_list args;

    va_start(args, format);

    set_terminal_color(colors[log_type]);

    printf(LOG_PREFIXS[log_type]);
    print(0, format, args);
    printf("\n");

    set_terminal_color(VGA_DEFAULT_COLOR);
}