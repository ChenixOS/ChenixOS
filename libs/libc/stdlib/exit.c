#include <stdlib.h>
#include "../internal/syscall.h"

void (* _atexit_tab[ATEXIT_MAX])(void);

int atexit(void (* func)(void))
{
    int i, res = -1;

    for (i = 0; i < ATEXIT_MAX; i++) {
        if (_atexit_tab[i] == NULL) {
            _atexit_tab[i] = func;
            res = 0;
            break;
        }
    }
    return res;
}

void exit(int status) {
    int i;

    for (i = ATEXIT_MAX-1; i >= 0; i--) {
        if (_atexit_tab[i] != NULL)
            _atexit_tab[i]();
    }

    syscall_invoke(syscall_exit,status,0,0,0);
}