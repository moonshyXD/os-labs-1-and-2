#ifndef STATUS_H
#define STATUS_H

typedef enum {
    STATUS_OK,
    STATUS_END_OF_INPUT,
    STATUS_IO_ERROR,
    STATUS_BAD_NUMBER,
    STATUS_NO_NUMBERS,
    STATUS_PIPE_ERROR
} Status;

#endif
