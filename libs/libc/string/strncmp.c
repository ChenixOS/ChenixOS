

#include <string.h>

int strncmp(const char *s1, const char *s2, size_t n)
{
    while ((*s1 != '\0') && (*s1 == *s2) && (n-- > 0)) {
        s1++;
        s2++;
    }
    return (n == 0) ? 0 : (*s1 - *s2);
}
