

#include <string.h>

char *strchr(const char *str, int c)
{
    while (*str != 0 && *str != c)
        str++;
    return (*str != 0) ? (char *)str : NULL;
}
