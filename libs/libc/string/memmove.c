#include <string.h>


void *memmove(void *dst, const void *src, size_t n)
{
    char *d = (char *) dst;
    char *s = (char *) src;
    size_t i;

    /*
     * Depending on the memory start locations, copy may be direct or
     * reverse, to avoid overwriting before the relocation is done.
     */
    if (d < s) {
        for (i = 0; i < n; i++)
            d[i] = s[i];
    } else { /* s <= d */
        i = n;
        while (i-- > 0)
            d[i] = s[i];
    }
    return dst;
}
