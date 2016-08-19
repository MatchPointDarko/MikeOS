/*
 * MikeOS: Helper functions for strings.
 */
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

#if 0
int strcmp(const char* str1, const char* str2)
{
    while(*str != '/0' && *str2 != '/0')
    {
        if(*str1 != *str2)
        {
            return *str1 - *str2;
        }
    }

    if(*str2 != '/0') return -1;

    if(*str1 != '/0') return 1;

    return 0;
}
#endif

