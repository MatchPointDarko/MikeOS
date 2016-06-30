#ifndef MIKE_OS_STDARG_H
#define MIKE_OS_STDARG_H

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

#endif //MIKE_OS_STDARG_H
