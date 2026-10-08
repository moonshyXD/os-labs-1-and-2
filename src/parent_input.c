#include "parent_input.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

static void strip_newline(char *line, ssize_t len) {
    if (len > 0 && line[len - 1] == '\n') {
        line[--len] = '\0';
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
        }
    }
}

static ssize_t read_input_line(char **line, size_t *capacity) {
    for (;;) {
        errno = 0;
        ssize_t length = getline(line, capacity, stdin);
        if (length >= 0 || errno != EINTR) {
            return length;
        }
        clearerr(stdin);
    }
}

static Status input_failure_status(void) {
    return feof(stdin) && !ferror(stdin)
        ? STATUS_END_OF_INPUT : STATUS_IO_ERROR;
}

Status read_filename(char **out) {
    *out = NULL;
    char *line = NULL;
    size_t cap = 0;
    ssize_t len = read_input_line(&line, &cap);
    if (len == -1) {
        Status status = input_failure_status();
        free(line);
        return status;
    }
    strip_newline(line, len);
    if (line[0] == '\0') {
        fprintf(stderr, "parent: пустое имя выходного файла\n");
        free(line);
        return STATUS_IO_ERROR;
    }
    *out = line;
    return STATUS_OK;
}

Status read_command_line(char **out, size_t *out_len) {
    *out = NULL;
    *out_len = 0;
    char *line = NULL;
    size_t cap = 0;
    ssize_t len = read_input_line(&line, &cap);
    if (len == -1) {
        Status status = input_failure_status();
        free(line);
        return status;
    }
    if ((len == 1 && line[0] == '\n') ||
        (len == 2 && line[0] == '\r' && line[1] == '\n')) {
        free(line);
        return STATUS_END_OF_INPUT;
    }
    *out = line;
    *out_len = (size_t)len;
    return STATUS_OK;
}
