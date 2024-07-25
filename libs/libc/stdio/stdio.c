#include <stdio.h>

int getchar(void)
{
    return fgetc(stdin);
}

char *gets(char *str)
{
    return fgets(str, INT_MAX, stdin);
}

int putchar(int c)
{
    return putc(c, stdout);
}

int puts(const char *str)
{
    return fputs(str, stdout);
}

