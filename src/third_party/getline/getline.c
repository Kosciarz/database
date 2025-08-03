/* The original code is public domain -- Will Hartung 4/9/2009 */
/* Modifications, public domain as well, by Antti Haapala, 11/10/2017
   - Switched to getc on 5/23/2019
   - Proper error handling on IO error and wraparound 
     check on buffer extension 3/31/2025 - 4/2/2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#define MINIMUM_BUFFER_SIZE 128

// if typedef doesn't exist (msvc, blah)
typedef intptr_t ssize_t;


ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = malloc(MINIMUM_BUFFER_SIZE);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = MINIMUM_BUFFER_SIZE;
    }

    pos = 0;
    while(c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);

            // have some reasonable minimum
            if (new_size < MINIMUM_BUFFER_SIZE) {
                new_size = MINIMUM_BUFFER_SIZE;
            }
            
            // size_t wraparound
            if (new_size <= *n) {
                errno = ENOMEM;
                return -1;
            }

            // Note you might also want to check that PTRDIFF_MAX
            // is not exceeded!

            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos ++] = c;
        if (c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    
    // if an IO error occurred, return -1
    if (c == EOF && !feof(stream)) {
        return -1;
    }

    // otherwise we successfully read until the end-of-file
    // or the delimiter
    return pos;
}