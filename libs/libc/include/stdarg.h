#ifndef _STDARG_H_
#define _STDARG_H_

/*
typedef unsigned char *va_list;

#define va_start(ap, last) \
    ((ap) = ((va_list) &(last)) + (sizeof (last)))

#define va_end(ap) \
    ((ap) = (va_list)0)

#define va_copy(dest, src) \
    ((dest) = (va_list) (src))

#define va_arg(ap, type) \
     (((ap) = (ap) + (sizeof (type))), *((type *) ((ap) - (sizeof (type)))))
*/

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

#endif /* _STDARG_H_ */