#ifndef _CTYPE_H_
#define _CTYPE_H_

/** Test for a blank character. */
#define isblank(c)  ((int) ((c) == ' ' || (c) == '\t'))

/** Test for a white-space character. */
#define isspace(c)  ((int) ((c) == ' '  || (c) == '\n' || (c) == '\f' \
                         || (c) == '\r' || (c) == '\t' || (c) == 'v'))

/** Test for a decimal digit. */
#define isdigit(c)  ((int) ((c) >= '0' && (c) <= '9'))

/** Test for a hexadecimal digit. */
#define isxdigit(c) ((int) (((c) >= '0' && (c) <= '9') \
                         || ((c) >= 'A' && (c) <= 'F') \
                         || ((c) >= 'a' && (c) <= 'f')))

/** Test for an uppercase letter. */
#define isupper(c)  ((int) ((c) >= 'A' && (c) <= 'Z'))

/** Test for a lowercase letter. */
#define islower(c)  ((int) ((c) >= 'a' && (c) <= 'z'))

/** Test for a control character. */
#define iscntrl(c)  ((int) (((c) >= 0x00 && (c) <= 0x1F) || (c) == 0x7F))

/** Test for a visible character. */
#define isgraph(c)  ((int) ((c) >= 0x21 && (c) <= 0x7E))

/** Test for a printable character. */
#define isprint(c)  ((int) ((c) >= 0x20 && (c) <= 0x7E))

/** Test for an alphabetic character. */
#define isalpha(c)  (isupper(c) || islower(c))

/** Test for an alphanumeric character. */
#define isalnum(c)  (isalpha(c) || isdigit(c))

/** Test for a punctuation character. */
#define ispunct(c)  (isgraph(c) && (!isspace(c)) && (!isalnum(c)))

/** Transliterate uppercase characters to lowercase. */
#define tolower(c)  (isupper(c) ? ((c) + 0x20) : (c))

/** Transliterate lowercase characters to uppercase. */
#define toupper(c)  (islower(c) ? ((c) - 0x20) : (c))

/** Translate an integer to a 7-bit ASCII character. */
#define toascii(c)  (c & 0x7F)

#endif /* _CTYPE_H_ */