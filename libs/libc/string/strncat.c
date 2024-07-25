

#include <string.h>

char *strncat(char *dst, const char *src, size_t n)
{
    size_t dst_len = strlen(dst);
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dst[dst_len + i] = src[i];
    dst[dst_len + i] = '\0';
    return dst;
}
