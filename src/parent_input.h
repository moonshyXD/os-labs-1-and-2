#ifndef PARENT_INPUT_H
#define PARENT_INPUT_H

#include <stddef.h>
#include "status.h"

Status read_filename(char **out);
Status read_command_line(char **out, size_t *out_len);

#endif
