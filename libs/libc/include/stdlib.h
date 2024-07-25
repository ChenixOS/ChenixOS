#ifndef __STDLIB_H__

#define __STDLIB_H__

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define ATEXIT_MAX  32

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Converts the initial portion of the string pointed to by str to int.
 *
 * @param str   Pointer to the string to convert.
 * @return      The converted value.
 */
int atoi(const char *str);

/**
 * Behaves the same as atoi(), except that they convert the initial portion of
 * the string to their return type of long.
 *
 * @param str   Pointer to the string to convert.
 * @return      The converted value.
 */
long atol(const char *str);

/**
 * Computes the absolute value of its integer operand, i.
 * If the result cannot be represented, the behavior is undefined.
 * @param i     Integer value value.
 * @return      The absolute value of i.
 */
int abs(int i);

// ==================== Memory ===========================
void* malloc(size_t size);

void free(void* block);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

// =======================================================

void exit(int status);

// ======================= Env ============================

char *getenv(const char *name);

int setenv(const char *name, const char *value, int rewrite);

int unsetenv(const char *name);

int putenv(char *str);

#ifdef __cplusplus
}
#endif

#endif