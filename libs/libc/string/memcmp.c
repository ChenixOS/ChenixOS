#include <string.h>

int memcmp(const void *s1, const void *s2, size_t n)
{
    const char *a = (const char *) s1;
    const char *b = (const char *) s2;
    size_t i;
    int res = 0;

    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            res = (int)a[i] - b[i];
            break;
        }
    }
    return res;
}
