#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdlib.h>

#include "getline.h"

typedef struct InputBuffer
{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer();
void free_input_buffer(InputBuffer* input_buffer);
void read_input(InputBuffer* input_buffer);

#endif // INPUT_H
