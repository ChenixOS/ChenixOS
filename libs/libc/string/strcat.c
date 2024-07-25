

#include <string.h>

char *strcat(char *dst, const char *src)
{
    size_t i;

    for (i = 0; dst[i] != 0; i++)
        ;
    strcpy(&dst[i], src);
    return dst;
}
