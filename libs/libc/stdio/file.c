#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FILE_FLAG_ERROR     (1 << 0)
#define FILE_FLAG_EOF       (1 << 1)
#define FILE_FLAG_NFREE     (1 << 2)

struct _FILE {
    int     fd;         /* file descriptor */
    int     flags;      /* mode of file access */
    int     bufmode;    /* buffering mode */
    int     bufsize;    /* buffer size */
    int     nr;         /* buffered read characters */
    int     nw;         /* buffered write characters */
    int     cnt;
    char    *ptr;       /* next character position */
    char    *buf;       /* location of buffer */
};

typedef struct _FILE FILE;

FILE stdio_streams[3] = {
    { 0, 0, _IOLBF, BUFSIZ, 0, 0, (char *)0, (char *)0 },
    { 1, 0, _IOLBF, BUFSIZ, 0, 0, (char *)0, (char *)0 },
    { 2, 0, _IONBF, 1, 0, 0, (char *)0, (char *)0 }
};

FILE *stdin  = &stdio_streams[0];
FILE *stdout = &stdio_streams[1];
FILE *stderr = &stdio_streams[2];

FILE *fopen(const char *path, const char *mode)
{
    int fd;
    FILE *fp;
    int flags;

    /* The 'b' letter is ignored on all POSIX conforming systems. */
    if (strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0) {
        flags = O_RDONLY;
    } else if (strcmp(mode, "r+") == 0 || strcmp(mode, "r+b") == 0 ||
               strcmp(mode, "rb+") == 0) {
        flags = O_RDWR;
    } else if (strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "w+") == 0 || strcmp(mode, "w+b") == 0 ||
               strcmp(mode, "wb+") == 0) {
        flags = O_RDWR | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0 || strcmp(mode, "ab") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (strcmp(mode, "a+") == 0 || strcmp(mode, "a+b") == 0 ||
               strcmp(mode, "ab+") == 0) {
        flags = O_RDWR | O_CREAT | O_APPEND;
    } else {
        errno = EINVAL;
        return NULL;
    }

    if ((fp = malloc(sizeof(*fp))) == NULL)
        return NULL;

    /* If O_CREAT is not specified then mode is ignored */
    if ((fd = open(path, flags, 0666)) < 0) {
        free(fp);
        return NULL;
    }
    fp->fd = fd;
    fp->flags = 0;
    fp->bufmode = _IOFBF;
    fp->bufsize = BUFSIZ;
    fp->nr = 0;
    fp->nw = 0;
    fp->buf = NULL;
    fp->ptr = NULL;
    return fp;
}

FILE *fdopen(int fd, const char *mode)
{
    FILE *fp;

#if 0
    int flags;

    /* The 'b' letter is ignored on all POSIX conforming systems. */
    if (!strcmp(mode, "r") || !strcmp(mode, "rb"))
        flags = O_RDONLY;
    else if (!strcmp(mode, "r+") || !strcmp(mode, "r+b")
            || !strcmp(mode, "rb+"))
        flags = O_RDWR;
    else if (!strcmp(mode, "w") || !strcmp(mode, "wb"))
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (!strcmp(mode, "w+") || !strcmp(mode, "w+b")
            || !strcmp(mode, "wb+"))
        flags = O_RDWR | O_CREAT | O_TRUNC;
    else if (!strcmp(mode, "a") || !strcmp(mode, "ab"))
        flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (!strcmp(mode, "a+") || !strcmp(mode, "a+b")
            || !strcmp(mode, "ab+"))
        flags = O_RDWR | O_CREAT | O_APPEND;
    else {
        errno = EINVAL;
        return NULL;
    }
#endif

    /* TODO: check if fd flags are compatible with flags (fcntl) ? */

    if ((fp = malloc(sizeof(*fp))) == NULL)
        return NULL;

    fp->fd = fd;
    fp->flags = 0;
    return fp;
}

int fclose(FILE *stream)
{
    if (stream->buf != NULL) {
        if (stream->nw != 0)
            fflush(stream);
        if ((stream->flags & FILE_FLAG_NFREE) == 0)
            free(stream->buf);
        stream->buf = NULL;
    }
    return close(stream->fd);
}

int feof(FILE *stream)
{
    return (stream->flags & FILE_FLAG_EOF);
}

int ferror(FILE *stream)
{
    return (stream->flags & FILE_FLAG_ERROR);
}

int fileno(FILE *stream)
{
    return stream->fd;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    int res = 0;

    if (stream->buf != NULL) {
        if (stream->cnt > 0)
            fflush(stream);
        free(stream->buf);
    }
    stream->bufmode = mode;
    stream->bufsize = size;
    stream->buf = buf;
    stream->ptr = buf;
    stream->cnt = 0;
}

// Begin: IO

int fflush(FILE *stream)
{
    int res = 0;
    ssize_t n;
    char *p = stream->buf;

    while (stream->nw > 0) {
        if ((n = write(stream->fd, p, stream->nw)) < 0) {
            stream->flags |= FILE_FLAG_ERROR;
            res = EOF;
            break;
        }
        if (n > stream->nw)
            n = stream->nw; /* be defensive */
        stream->nw -= n;
        p += n;
    }
    stream->ptr = stream->buf;
    stream->nw = 0;
    stream->nr = 0;
    return res;
}

static int _fillbuf(FILE *stream)
{
    if (stream->buf == NULL) {
        if ((stream->buf = (char *)malloc(stream->bufsize)) == NULL)
            return EOF;
    }
    stream->ptr = stream->buf;
    stream->nr = read(stream->fd, stream->ptr, stream->bufsize);
    if (--stream->nr < 0) {
        if (stream->nr == -1)
            stream->flags |= FILE_FLAG_EOF;
        else
            stream->flags |= FILE_FLAG_ERROR;
        stream->nr = 0;
        stream->nw = 0;
        return EOF;
    }
    return (unsigned char) *stream->ptr++;
}

int fgetc(FILE *stream)
{
    if (stream->nw > 0)
        fflush(stream);
    return (--(stream)->nr >= 0) ?
        (unsigned char)*stream->ptr++ : _fillbuf(stream);
}

char *fgets(char *str, int size, FILE *fp)
{
    int i; /* index inside string buffer. */

    for (i = 0; i < (size-1); i++) {
        str[i] = fgetc(fp);
        if (str[i] == EOF || str[i] == '\n')
            break;
    }

    /* if `i` is zero, nothing was read and `NULL` is returned. */
    if (i == 0)
        return NULL;

    str[i] = '\0'; /* replace EOF or '\n' with null terminator */
    return str;
}

int fputc(int c, FILE *stream)
{
    if (stream->buf == NULL) {
        stream->buf = (char *)malloc(stream->bufsize);
        if (stream->buf == NULL)
            return EOF;
        stream->nr = 0;
        stream->nw = 0;
        stream->ptr = stream->buf;
    }
    if (stream->nr > 0) {
        lseek(stream->fd, SEEK_CUR, -stream->nr);
        stream->nr = 0;
        stream->nw = 0;
        stream->ptr = stream->buf; 
    }
    
    *stream->ptr++ = c;
    stream->nw++;
    if (stream->nw >= stream->bufsize ||
        (stream->bufmode == _IOLBF && (char)c == '\n') ||
        stream->bufmode == _IONBF) {
        if (fflush(stream) != 0)
            c = EOF;
    }
    return (unsigned char)c;
}

int fputs(const char *str, FILE *stream)
{
    int i, status;

    for (i = 0; i < strlen(str); i++) {
        status = fputc(str[i], stream);
        if (status == EOF)
            break;
    }
    return status;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n, s;
    char *buf = (char *)ptr;
    int c;
    int stop = 0;

    n = 0;
    while (n < nmemb && stop == 0) {
        s = 0;
        while (s < size) {
            if ((c = fgetc(stream)) == EOF)
                return n;
            buf[s++] = (unsigned char)c;
            if (stream->bufmode == _IOLBF && c == '\n') {
                stop = 1;
                break;
            }
        }
        buf += size;
        n++;
    }
    return n;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t n, s;
    const char *buf = (const char *)ptr;

    n = 0;
    while (n < nmemb) {
        s = 0;
        while (s < size) {
            if (fputc(buf[s], stream) == EOF)
                return n;
            s++;
        }
        buf += size;
        n++;
    }
    return n;
}



