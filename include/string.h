#ifndef MIKE_OS_STRING_H
#define MIKE_OS_STRING_H

#include "common.h"

char* strcpy(char* dest, const char* source);
char* strncpy(char* dest, const char* source, int num);
int strcmp(const char* str1, const char* str2);
char* strtok(char* str, const char* delimiters);
char* strchr(char* str1, int character);
uint32_t strspn(const char* str1, const char* str2);
const char* strpbrk(const char* str1, const char* str2);
int count_chars(const char* str, int character);
char* strstr(const char* haystack, const char* needle);
uint32_t strlen(char* str);

#endif //MIKE_OS_STRING_H
