// syscalls.c
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/* Heap management for _sbrk */
caddr_t _sbrk(int incr) {
    extern char _end;
    static char *heap_end;
    char *prev_heap_end;

    if (!heap_end)
        heap_end = &_end;
    prev_heap_end = heap_end;
    heap_end += incr;

    return (caddr_t) prev_heap_end;
}

/* Exit */
void _exit(int status) {
    (void)status;
    while (1);  // loop forever
}

/* Write to file descriptor (stdout, stderr) */
int _write(int file, const char *ptr, int len) {
    (void)file; (void)ptr; (void)len;
    return -1;
}

/* Read from file descriptor */
int _read(int file, char *ptr, int len) {
    (void)file; (void)ptr; (void)len;
    errno = EBADF;
    return -1;
}

/* Close file descriptor */
int _close(int file) {
    (void)file;
    errno = EBADF;
    return -1;
}

/* fstat */
int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR; // pretend character device
    return 0;
}

/* lseek */
int _lseek(int file, int ptr, int dir) {
    (void)file; (void)ptr; (void)dir;
    errno = EBADF;
    return -1;
}

/* isatty */
int _isatty(int file) {
    (void)file;
    return 1;  // pretend all file descriptors are tty
}