#include "getline.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define MINIMUM_BUFFER_SIZE 128

ssize_t getline(char** lineptr, size_t* n, FILE* stream)
{
    if (lineptr == NULL || n == NULL || stream == NULL)
        return -1;
    
    int ch = getc(stream);
    if (ch == EOF)
        return -1;

    if (*lineptr == NULL)
    {
        *lineptr = malloc(MINIMUM_BUFFER_SIZE);
        if (*lineptr == NULL)
            return -1;

        *n = MINIMUM_BUFFER_SIZE;
    }

    size_t pos = 0;
    while (ch != EOF)
    {
        if (pos + 1 > *n)
        {
            size_t new_size = *n * 2;
            if (new_size < MINIMUM_BUFFER_SIZE)
                new_size = MINIMUM_BUFFER_SIZE;

            // size_t wraparound
            if (new_size <= *n)
                return -1;
        
            char* new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL)
                return -1;
        
            *lineptr = new_ptr;
            *n = new_size;
        }

        (*lineptr)[pos ++] = ch;
        if (ch == '\n')
            break;

        ch = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    
    if (ch == EOF && !feof(stream))
        return -1;

    return pos;
}
