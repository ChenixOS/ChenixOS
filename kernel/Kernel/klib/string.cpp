#include "string.h"
#include "types.h"
#include "kernel/SymbolTable.h"

int kstrcmp(const char* a, const char* b)
{
    int i = 0;
    while(a[i] == b[i]) {
        if(a[i] == '\0')
            return 0;
        i++;
    }
    return 1;
}

EXPORT_DEF_SYMBOL(kstrcmp)

int kstrlen(const char* a)
{
    int l = 0;
    while(a[l] != '\0')
        l++;
    return l;
}
EXPORT_DEF_SYMBOL(kstrlen)

void kstrcpy(char* dest, const char* src)
{
    int i = 0;
    while(src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}
EXPORT_DEF_SYMBOL(kstrcpy);

char *kstrncpy(char *dst, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dst[i] = src[i];
    for ( ; i < n; i++)
        dst[i] = '\0';
    return dst;
}
EXPORT_DEF_SYMBOL(kstrncpy);

void kstrconcat(char* dest, const char* a, const char* b)
{
    while(*a != '\0') {
        *dest = *a;
        dest++;
        a++;
    }
    while(*b != '\0') {
        *dest = *b;
        dest++;
        b++;
    }
    *dest = '\0';
}
EXPORT_DEF_SYMBOL(kstrconcat);


int kstrncmp(const char *s1, const char *s2, size_t n)
{
    while ((*s1 != '\0') && (*s1 == *s2) && (n-- > 0)) {
        s1++;
        s2++;
    }
    return (n == 0) ? 0 : (*s1 - *s2);
}
EXPORT_DEF_SYMBOL(kstrncmp);


// 辅助函数：反转字符串
char* reverse(char* str, size_t len) {
    char temp;
    for (size_t i = 0; i < len / 2; i++) {
        temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    return str;
}

// itoa 函数实现
char* itoa(int value, char* str, int base) {
    int i = 0;
    bool isNegative = false;

    // 处理负数情况
    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }

    // 处理0的情况
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        if (isNegative) {
            // 如果需要，添加负号
            str[i++] = '-';
            str[i] = '\0';
        }
        return str;
    }

    // 转换数字到字符串
    while (value) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    // 如果数字是负数，添加负号
    if (isNegative) {
        str[i++] = '-';
    }

    // 添加字符串终止符
    str[i] = '\0';

    // 反转字符串，因为我们是从低位到高位生成的
    if (base == 10) {
        reverse(str, isNegative ? i - 1 : i);
    }

    return str;
}
EXPORT_DEF_SYMBOL(itoa);