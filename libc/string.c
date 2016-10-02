/*
 * MikeOS: Helper functions for strings.
 */

#include <common.h>
#include <libc/string.h>

char* strcpy(char* dest, const char* source)
{
    while((*dest++ = *source++) != '\0');
    return dest;
}

char* strncpy(char* dest, const char* source, int num)
{
    while((*dest++ = *source++) != '\0' && num-- > 0);
    return dest;
}

int strncmp(char* str1, char* str2, uint32_t max_length)
{
    str1[max_length - 1] = '\0';
    str2[max_length - 1] = '\0';

    return strcmp((const char*)str1, (const char*)str2);
}

int strcmp(const char* str1, const char* str2)
{
    while(*str1 != '\0' && *str2 != '\0')
    {
        if(*str1 != *str2)
        {
            return *str1 - *str2;
        }

        str1++;
        str2++;
    }

    if(*str2 != '\0') return -1;

    if(*str1 != '\0') return 1;

    return 0;
}

static bool_t char_in_string(const char character, const char* str)
{
    while(*str != '\0')
    {
        if(*str == character)
        {
            return true;
        }
        str++;
    }

    return false;
}

const char* strpbrk(const char* str1, const char* str2)
{
    while(*str1 != '\0')
    {
        if(char_in_string(*str1, str2))
        {
            return str1;
        }

        str1++;
    }

    return NULL;
}

uint32_t strspn(const char* str1, const char* str2)
{
    uint32_t number_of_chars = 0;

    while(*str1 != '\0')
    {
        if(!char_in_string(*str1, str2))
        {
            break;
        }

        str1++;
        number_of_chars++;
    }

    return number_of_chars;
}

char* strchr(char* str, int character)
{
    while(*str != '\0')
    {
        if (*str == character)
        {
            return str;
        }

        str++;
    }

    return NULL;
}

char* strtok(char* str, const char* delimiters)
{
    static char* old_str;
    int had_chars = 0;

    if (str == NULL)
    {
        str = old_str;
    }

    str += strspn(str, delimiters);

    if(*str == '\0')
    {
        return NULL;
    }

    char* token = (char *)strpbrk(str, delimiters);

    if(token == NULL)
    {
        old_str = strchr(str, '\0');
    }

    else
    {
        *token = '\0';
        old_str = token + 1;
    }

    return str;
}

uint32_t count_chars(const char* str, int character)
{
    uint32_t count = 0;

    while(*str != '\0')
    {
        if(*str == character)
        {
            count++;
        }
    }

    return count;
}

char* strstr(const char* haystack, const char* needle)
{
    if(haystack == NULL || needle == NULL)
    {
        return NULL;
    }

    //O(n) :-)
    //n = strlen(hay_stack)

    const char* needle_iter = needle;
    const char* substr = NULL;

    while(*haystack != '\0' && *needle != '\0')
    {
        if(*needle_iter == *haystack)
        {
            if(substr == NULL)
            {
                substr = haystack;
            }
            needle_iter++;
        }
        else
        {
            substr = NULL;
            needle_iter = needle;
        }
    }

    return (char *)substr;
}

uint32_t strlen(const char* str)
{
    uint32_t counter = 0;
    while(*str++ != '\0')
    {
        counter++;
    }

    return counter;
}
