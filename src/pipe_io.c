#include "pipe_io.h"
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

int write_all(int fd, const char *buf, size_t len) {
    size_t written = 0;
    while (written < len) {
        size_t remaining = len - written;
        size_t chunk = remaining > (size_t)SSIZE_MAX ? (size_t)SSIZE_MAX : remaining;
        ssize_t n = write(fd, buf + written, chunk);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            errno = EIO;
            return -1;
        }
        written += (size_t)n;
    }
    return 0;
}

static int ensure_capacity(char **line, size_t *cap, size_t needed) {
    if (needed <= *cap) {
        return 0;
    }
    size_t new_cap = *cap == 0 ? 64 : *cap;
    while (new_cap < needed) {
        if (new_cap > SIZE_MAX / 2) {
            new_cap = needed;
            break;
        }
        new_cap *= 2;
    }
    char *grown = realloc(*line, new_cap);
    if (grown == NULL) {
        return -1;
    }
    *line = grown;
    *cap = new_cap;
    return 0;
}

ssize_t read_line(int fd, char **line, size_t *cap) {
    size_t len = 0;
    size_t consumed = 0;
    for (;;) {
        char c;
        ssize_t n = read(fd, &c, 1);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            break;
        }
        if (consumed == (size_t)SSIZE_MAX) {
            errno = EOVERFLOW;
            return -1;
        }
        consumed++;
        if (c == '\n') {
            break;
        }
        if (c == '\0') {
            errno = EINVAL;
            return -1;
        }
        if (ensure_capacity(line, cap, len + 2) == -1) {
            return -1;
        }
        (*line)[len++] = c;
    }
    if (ensure_capacity(line, cap, len + 1) == -1) {
        return -1;
    }
    (*line)[len] = '\0';
    return (ssize_t)consumed;
}
