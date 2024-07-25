#pragma once

#include "types.h"

int kstrcmp(const char* a, const char* b);

int kstrcmp(const char* a, int aStart, int aEnd, const char* b);

int kstrlen(const char* a);

void kstrcpy(char* dest, const char* src);

char *kstrncpy(char *dst, const char *src, size_t n);

void kstrconcat(char* dest, const char* a, const char* b);

int kstrncmp(const char *s1, const char *s2, size_t n);

char* itoa(int value, char* str, int base);