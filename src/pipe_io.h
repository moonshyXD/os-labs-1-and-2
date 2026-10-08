#ifndef PIPE_IO_H
#define PIPE_IO_H

#include <stddef.h>
#include <sys/types.h>

int write_all(int fd, const char *buf, size_t len);
ssize_t read_line(int fd, char **line, size_t *cap);

#endif
