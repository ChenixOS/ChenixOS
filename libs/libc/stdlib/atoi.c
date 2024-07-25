#include <stdlib.h>
#include <ctype.h>

int atoi(const char *str)
{
    int i;
    int sign = +1;
    int num;

    for (i = 0; isspace(str[i]); i++)
        ;

    if (str[i] == '+') {
        i++;
    } else if (str[i] == '-') {
        sign = -1;
        i++;
    }

    for (num = 0; isdigit(str[i]); i++) {
        num = (num << 3)+(num << 1);    /* num *= 10 */
        num += (str[i] - '0');
    }

    if (sign == -1)
        num = -num;
    return num;
}

long atol(const char *str)
{
    int i;
    int sign = +1;
    long num;

    for (i = 0; isspace(str[i]); i++)
        ;

    if (str[i] == '+') {
        i++;
    } else if (str[i] == '-') {
        sign = -1;
        i++;
    }

    for (num = 0; isdigit(str[i]); i++) {
        num = (num << 3)+(num << 1);    /* num *= 10 */
        num += (str[i] - '0');
    }

    if (sign == -1)
        num = -num;
    return num;
}