#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pipe_io.h"
#include "status.h"
#include "sum.h"

static void report_error(const char *what) {
    fprintf(stderr, "%s: %s\n", what, strerror(errno));
}

static void report(Status status) {
    switch (status) {
        case STATUS_OK:
        case STATUS_END_OF_INPUT:
            break;
        case STATUS_IO_ERROR:
        case STATUS_PIPE_ERROR:
            fprintf(stderr, "child: ошибка ввода-вывода\n");
            break;
        case STATUS_BAD_NUMBER:
        case STATUS_NO_NUMBERS:
            break;
    }
}

static int write_sum(int fd, long sum) {
    char output[sizeof(long) * CHAR_BIT + 2];
    int out_len = snprintf(output, sizeof(output), "%ld\n", sum);
    if (out_len < 0 || (size_t)out_len >= sizeof(output)) {
        fprintf(stderr, "child: не удалось отформатировать сумму\n");
        return EXIT_FAILURE;
    }
    if (write_all(fd, output, (size_t)out_len) == -1) {
        report_error("write");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int process_lines(int fd) {
    char *line = NULL;
    size_t cap = 0;
    ssize_t len;
    int exit_status = 0;
    while ((len = read_line(STDIN_FILENO, &line, &cap)) > 0) {
        Status status = STATUS_OK;
        long sum = sum_line(line, &status);
        if (status != STATUS_OK) {
            report(status);
            if (status == STATUS_NO_NUMBERS) {
                continue;
            }
            exit_status = 1;
            if (status != STATUS_BAD_NUMBER) {
                break;
            }
            continue;
        }

        if (write_sum(fd, sum) != EXIT_SUCCESS) {
            exit_status = EXIT_FAILURE;
            break;
        }
    }
    free(line);
    if (len == -1) {
        report_error("read_line");
        exit_status = 1;
    }
    return exit_status;
}

static int run_child(const char *filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        report_error("open");
        return EXIT_FAILURE;
    }
    int exit_status = process_lines(fd);
    if (close(fd) == -1) {
        report_error("close");
        exit_status = 1;
    }
    return exit_status;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <output_filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argv[1][0] == '\0') {
        fprintf(stderr, "child: пустое имя выходного файла\n");
        return EXIT_FAILURE;
    }
    return run_child(argv[1]);
}
