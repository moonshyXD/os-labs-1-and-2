#include "sum.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long sum_line(const char *line, Status *status) {
    char *copy = strdup(line);
    if (copy == NULL) {
        *status = STATUS_IO_ERROR;
        return 0;
    }

    long sum = 0;
    int had_number = 0;
    *status = STATUS_OK;

    char *saveptr = NULL;
    char *tok = strtok_r(copy, " \t\r\n\v\f", &saveptr);
    while (tok != NULL) {
        char *end = NULL;
        errno = 0;
        long value = strtol(tok, &end, 10);
        if (end == tok || *end != '\0' || errno == ERANGE ||
            value < INT_MIN || value > INT_MAX) {
            const char *reason = end == tok || *end != '\0'
                ? "не число" : "число вне диапазона int";
            fprintf(stderr, "child: %s: \"%s\"\n", reason, tok);
            *status = STATUS_BAD_NUMBER;
        } else if ((value > 0 && sum > LONG_MAX - value) ||
                   (value < 0 && sum < LONG_MIN - value)) {
            fprintf(stderr, "child: переполнение суммы\n");
            *status = STATUS_BAD_NUMBER;
        } else {
            sum += value;
            had_number = 1;
        }
        tok = strtok_r(NULL, " \t\r\n\v\f", &saveptr);
    }

    free(copy);
    if (!had_number && *status == STATUS_OK) {
        *status = STATUS_NO_NUMBERS;
    }
    return sum;
}
