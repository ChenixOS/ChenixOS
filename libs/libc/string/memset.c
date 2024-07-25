
#include <string.h>


void *memset(void *s, int c, size_t n)
{
    char *d = (char *) s;
    int i;

    for (i=0; i < n; i++)
        d[i] = c;
    return s;
}
